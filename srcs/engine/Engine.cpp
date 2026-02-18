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
