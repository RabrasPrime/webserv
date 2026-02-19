//
// Created by tjooris on 2/16/26.
//

#include "Engine.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>

class Listener;
class Server;
class Client;

Engine::Engine() : _epoll_fd(-1), _is_running(false) {}

Engine::~Engine()
{
    stop();
    if (_epoll_fd != -1)
    {
        close(_epoll_fd);
        _epoll_fd = -1;
    }
}

static std::string make_listener_key(int host, int port)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%d:%d", host, port);
    return std::string(buf);
}

static bool extract_host_port(const struct sockaddr_storage& addr, int& host, int& port)
{
    if (addr.ss_family == AF_INET)
    {
        const struct sockaddr_in* a = reinterpret_cast<const struct sockaddr_in*>(&addr);
        host = ntohl(a->sin_addr.s_addr);
        port = ntohs(a->sin_port);
        return true;
    }
    else if (addr.ss_family == AF_INET6)
    {
        host = 0;
        const struct sockaddr_in6* a = reinterpret_cast<const struct sockaddr_in6*>(&addr);
        port = ntohs(a->sin6_port);
        return true;
    }
    return false;
}

void Engine::init_listeners()
{
    std::map<std::string, Listener> tmp_listeners;

    for (size_t i = 0; i < _servers_config.size(); i++)
    {
        Server& server = _servers_config[i];
        const std::vector<struct sockaddr_storage>& addrs = server.get_addr();

        for (size_t j = 0; j < addrs.size(); j++)
        {
            int host, port;
            if (!extract_host_port(addrs[j], host, port))
            {
                std::cerr << "Unknown address family, skipping" << std::endl;
                continue;
            }
            std::string key = make_listener_key(host, port);
            if (tmp_listeners.find(key) == tmp_listeners.end())
                tmp_listeners[key] = Listener(host, port);

            tmp_listeners[key].add_server(&server);
        }
    }
    for (std::map<std::string, Listener>::iterator it = tmp_listeners.begin();
         it != tmp_listeners.end(); ++it)
    {
        Listener& listener = it->second;
        if (!listener.create_socket())
        {
            std::cerr << "Failed to create socket for listener" << std::endl;
            continue;
        }
        if (!listener.bind_socket())
        {
            std::cerr << "Failed to bind socket" << std::endl;
            listener.close_socket();
            continue;
        }
        if (!listener.listen_socket())
        {
            std::cerr << "Failed to listen on socket" << std::endl;
            listener.close_socket();
            continue;
        }
        int fd = listener.get_fd();
        _listeners[fd] = listener;
        _fd_types[fd] = FD_LISTENER;
    }
}



void Engine::setup_epoll()
{
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0)
    {
        std::cerr << "Error creating epoll fd: " << strerror(errno) << std::endl;
        return;
    }

    for (std::map<int, Listener>::iterator it = _listeners.begin();
         it != _listeners.end(); ++it)
    {
        add_to_epoll(it->first, EPOLLIN);
    }
}

void Engine::stop()
{
    _is_running = false;

    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        it->second.close();
    _clients.clear();
    for (std::map<int, Listener>::iterator it = _listeners.begin(); it != _listeners.end(); ++it)
        it->second.close_socket();
    _listeners.clear();
    _fd_types.clear();
}

bool Engine::is_running() const
{
    return _is_running;
}


void Engine::handle_new_connection(int listener_fd)
{
    Listener& listener = _listeners[listener_fd];
    int client_fd = listener.accept_connection();

    if (client_fd < 0)
        return;
    Server* server = listener.get_servers().empty() ? NULL : listener.get_servers()[0];

    _clients[client_fd] = Client(client_fd, server);
    _fd_types[client_fd] = FD_CLIENT;
    add_to_epoll(client_fd, EPOLLIN | EPOLLET);
}

void Engine::handle_client_read(int client_fd)
{
    std::map<int, Client>::iterator it = _clients.find(client_fd);
    if (it == _clients.end())
        return;

    Client& client = it->second;
    ssize_t ret = client.read_from_socket();

    if (ret == 0)
    {
        handle_client_disconnect(client_fd);
        return;
    }
    if (ret < 0)
    {
        handle_client_disconnect(client_fd);
        return;
    }

    //Sammy, c'est ta partie, pour l'instant j'ai mis une commande random de gemini pour l'instant
    client.get_write_buffer() = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
    modify_epoll(client_fd, EPOLLOUT | EPOLLET);
}

void Engine::handle_client_write(int client_fd)
{
    std::map<int, Client>::iterator it = _clients.find(client_fd);
    if (it == _clients.end())
        return;

    Client& client = it->second;
    ssize_t ret = client.write_to_socket();

    if (ret < 0)
    {
        handle_client_disconnect(client_fd);
        return;
    }

    if (client.get_write_buffer().empty())
    {
        modify_epoll(client_fd, EPOLLIN | EPOLLET);
    }
}

void Engine::handle_client_disconnect(int client_fd)
{
    remove_from_epoll(client_fd);
    _fd_types.erase(client_fd);

    std::map<int, Client>::iterator it = _clients.find(client_fd);
    if (it != _clients.end())
    {
        it->second.close();
        _clients.erase(it);
    }
}

void Engine::add_to_epoll(int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        std::cerr << "epoll_ctl ADD error for fd " << fd << ": "
                  << strerror(errno) << std::endl;
    }
}

void Engine::modify_epoll(int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0)
    {
        std::cerr << "epoll_ctl MOD error for fd " << fd << ": "
                  << strerror(errno) << std::endl;
    }
}

void Engine::remove_from_epoll(int fd)
{
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
    {
        std::cerr << "epoll_ctl DEL error for fd " << fd << ": "
                  << strerror(errno) << std::endl;
    }
}

void Engine::add_server(const Server& server)
{
    _servers_config.push_back(server);
}

Server* Engine::match_server(const std::string& host_header)
{
    for (std::map<int, Listener>::iterator it = _listeners.begin();
         it != _listeners.end(); ++it)
    {
        Server* s = it->second.match_server(host_header);
        if (s)
            return s;
    }
    return NULL;
}
