//
// Created by tjooris on 2/16/26.
//

#include "Engine.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Client.hpp"
#include "Color.hpp"
#include "httpRequest.hpp"

class Listener;
class Server;
class Client;

Engine* Engine::_instance = NULL;

Engine::Engine() : _epoll_fd(-1), _is_running(false)
{
    _instance = this;
}

Engine::~Engine()
{
    stop();
    if (_epoll_fd != -1)
    {
        close(_epoll_fd);
        _epoll_fd = -1;
    }
}

void Engine::signal_handler(int sig)
{
	(void)sig;
	if (_instance)
		_instance->_is_running = false;
}

static std::string make_listener_key(const int host, const int port)
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
                tmp_listeners[key] = Listener(host, port, addrs[j]);
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

bool Engine::is_running() const
{
    return _is_running;
}


void Engine::handle_new_connection(int listener_fd)
{
    const Listener& listener = _listeners[listener_fd];
    const int client_fd = listener.accept_connection();

    if (client_fd < 0)
	{
        return ;
	}

	std::vector<Server *> ListServer = listener.get_servers();
    _clients[client_fd] = Client(client_fd, ListServer);
	_clients[client_fd].engine = this;
    _fd_types[client_fd] = FD_CLIENT;
    add_to_epoll(client_fd, EPOLLIN | EPOLLET);
}


std::string extractCookie(const std::string &cookie, const std::string &key)
{
    std::string toFind = key + "=";
    size_t start = cookie.find(toFind);

    if (start == std::string::npos)
        return "";
    start += toFind.length();
    size_t end = cookie.find(';', start);
    if (end == std::string::npos)
        return cookie.substr(start);
    return cookie.substr(start, end - start);
}

void Engine::handle_client_read(const int client_fd)
{
    const std::map<int, Client>::iterator it = _clients.find(client_fd);
    if (it == _clients.end())
        return;

    Client& client = it->second;
    const ssize_t ret = client.read_from_socket();
    if (ret == 0)
    {
        handle_client_disconnect(client_fd);
        return;
    }
	if (client.req.ErrorCode == 413)
	{
		std::vector<unsigned char>::iterator it;
		std::vector<unsigned char>& vect = client.get_read();
		for (it = vect.begin();it != vect.end();it++)
		{
			if (is_end_head(it, vect))
			{
				vect.erase(vect.begin(),it + 4);
				modify_epoll(client_fd, EPOLLOUT | EPOLLET);
				return;
			}
		}
		std::string tmp(vect.begin(),vect.end());
		return;
	}

	if (!client.req.end_head)
	{
		std::string header;
		std::vector<unsigned char>& vect = client.get_read();
		std::vector<unsigned char>::iterator it;
		for (it = vect.begin();it != vect.end();it++)
		{
			if (is_end_head(it, vect))
			{
				client.req.end_head = 1;
				parse_header(header, client.req, client.get_server());
				vect.erase(vect.begin(), it + 4);
				break;
			}
			else
				header += *it;
		}
		if (it == vect.end())
			return;
        if (client.req.mult.count("Cookie") > 0 && !client.req.mult["Cookie"].empty())
            std::string user = extractCookie(client.req.mult["Cookie"].front(), "user_id");
    
		if (client.req.chunked > 0 || (client.req.mult.find("Transfer-Encoding") != client.req.mult.end() && client.req.mult["Transfer-Encoding"].size() != 0 && client.req.mult["Transfer-Encoding"].front() == "chunked"))
		{
			handle_chunked(vect, client, client_fd);
			return;
		}
		else if (client.req.mult.find("Content-Length") != client.req.mult.end())
		{
			if (client.req.mult["Content-Length"].size() > 2)
			{
				client.req.body = client.get_read();
				client._write_buffer = client.res.handleResponse(client.req, 400);
				client.get_read().clear();
				client.write_to_socket();
				handle_client_disconnect(client_fd);
				return ;
			}
			std::stringstream ss(client.req.mult["Content-Length"].front());
			int ContentLength;
			if (!(ss >> ContentLength))
			{
				client.req.body = client.get_read();
				client._write_buffer = client.res.handleResponse(client.req, 400);
				client.get_read().clear();
				client.write_to_socket();
				handle_client_disconnect(client_fd);
			}
			client.req.ContentLength = ContentLength;

			if (client.get_read().size() != client.req.ContentLength)
				return ;
			else
			{
				client.req.body = client.get_read();
				client.req.chunked_size = -1;
				std::string resp = client.res.handleResponse(client.req, client.req.ErrorCode);
                if (resp == "START_CGI")
                {
					size_t i = 0;
					std::stringstream ss;
					ss << i;
					client.req.tmpName = "/tmp/" + ss.str() + ".tmp" ;
					struct stat st;
					while (stat(client.req.tmpName.c_str(), &st) != -1)
					{
						i++;
						ss.str("");
						ss.clear();
						ss << i;
						client.req.tmpName = "/tmp/" + ss.str() + ".tmp" ;
					}
					client.req.fd = open(client.req.tmpName.c_str(), O_CREAT | O_WRONLY, 6044);
					write(client.req.fd, &client.req.body[0], client.req.body.size());
					close(client.req.fd);
					client.req.chunked_size = 0;
					client.res.handleResponse(client.req, 1);

					int pipefd = client.req.pipeOut[0];
					_fd_types[pipefd] = FD_CGI_PIPE;
					_map_cgi_pid[pipefd] = client.req.cgi_pid;
					_cgi_to_client[pipefd] = client_fd;
					add_to_epoll(pipefd, EPOLLIN);
					close(client.req.pipeOut[1]);
					return;
                }
                client._write_buffer = resp;
				client.get_read().clear();
				modify_epoll(client_fd, EPOLLOUT | EPOLLET);
			}
		}
		else
		{
			client.req.body = client.get_read();
			client.req.chunked_size = -1;
			std::string resp = client.res.handleResponse(client.req, client.req.ErrorCode);
			if (resp == "START_CGI")
			{
				size_t i = 0;
				std::stringstream ss;
				ss << i;
				client.req.tmpName = "/tmp/" + ss.str() + ".tmp" ;
				struct stat st;
				while (stat(client.req.tmpName.c_str(), &st) != -1)
				{
					i++;
					ss.str("");
					ss.clear();
					ss << i;
					client.req.tmpName = "/tmp/" + ss.str() + ".tmp" ;
				}
				client.req.chunked_size = 0;
				client.res.handleResponse(client.req, 1);

				int pipefd = client.req.pipeOut[0];
				_fd_types[pipefd] = FD_CGI_PIPE;
				_map_cgi_pid[pipefd] = client.req.cgi_pid;
				_cgi_to_client[pipefd] = client_fd;
				add_to_epoll(pipefd, EPOLLIN);
				close(client.req.pipeOut[1]);
				return;
			}
            client._write_buffer = resp;
			client.get_read().clear();
			modify_epoll(client_fd, EPOLLOUT | EPOLLET);
		}
	}
	else
	{
		if (client.req.chunked != 0 || (client.req.mult.find("Transfer-Encoding") != client.req.mult.end() && client.req.mult["Transfer-Encoding"].size() != 0 && client.req.mult["Transfer-Encoding"].front() == "chunked"))
		{
			handle_chunked(client.get_read(), client, client_fd);
			return;
		}
		if (client.get_read().size() != client.req.ContentLength)
			return ;
		client.req.body = client.get_read();
		client.get_read().clear();
		std::string resp = client.res.handleResponse(client.req, client.req.ErrorCode);
		if (resp == "START_CGI")
			return ;
        client._write_buffer = resp;
        modify_epoll(client_fd, EPOLLOUT | EPOLLET);
	}
}

void Engine::handle_client_write(const int client_fd)
{
    const std::map<int, Client>::iterator it = _clients.find(client_fd);
	
    if (it == _clients.end())
        return;

    Client& client = it->second;
    const ssize_t ret = client.write_to_socket();

	client._write_buffer.clear();
	client.req.end_head = 0;
	client.req = HttpRequest();
	client.req.engine = this;
	client.res = httpResponse();

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

void Engine::handle_client_disconnect(const int client_fd)
{
    remove_from_epoll(client_fd);
    _fd_types.erase(client_fd);

    const std::map<int, Client>::iterator it = _clients.find(client_fd);
    if (it != _clients.end())
    {
        it->second.close();
        _clients.erase(it);
    }
}

void Engine::add_to_epoll(const int fd, const uint32_t events) const
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

void Engine::modify_epoll(const int fd, const uint32_t events) const
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

void Engine::remove_from_epoll(const int fd) const
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

void send_data(int fd, unsigned char *str, ssize_t size)
{
	ssize_t total_send = 0;
	while (total_send != size)
	{
		ssize_t bytes_send = send(fd, &str[total_send], size - total_send, 0);
		if (bytes_send != -1)
			total_send += bytes_send;
	}
}

void send_data(int fd, std::string str, ssize_t size)
{
	ssize_t total_send = 0;
	while (total_send != size)
	{
		ssize_t bytes_send = send(fd, &str[total_send], size - total_send, 0);
		if (bytes_send != -1)
			total_send += bytes_send;
	}
}

void Engine::run()
{
    if (_epoll_fd < 0)
    {
        std::cerr << "Epoll not set up, cannot run engine" << std::endl;
        return;
    }
    _is_running = true;
    struct epoll_event events[MAX_EVENTS];
    while (_is_running)
    {
        const int n = epoll_wait(_epoll_fd, events, MAX_EVENTS, 1000);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "Error in epoll_wait: " << strerror(errno) << std::endl;
            stop();
            break;
        }

        for (int i = 0; i < n; i++)
        {
			int fd = events[i].data.fd;

			if (_fd_types[fd] == FD_CGI_PIPE_IN)
			{
				int client_fd = _cgi_to_client[fd];
				const std::map<int, Client>::iterator it = _clients.find(client_fd);
				if (it == _clients.end())
					return;

				Client& client = it->second;
				char buffer[4096] = {0};
				size_t bytes_read = read(client.req.fd,buffer,sizeof(buffer));
				if (bytes_read > 0)
				{
					std::string str(buffer,bytes_read);
					write(fd, &str[0], str.size());
				}
				else
				{
					if (bytes_read == 0)
					{
						remove_from_epoll(fd);
						close(fd);
						close(client.req.fd);
						_fd_types.erase(fd);
						_cgi_to_client.erase(fd);
					}
				}
				continue;
			}

            if (_fd_types[fd] == FD_CGI_PIPE)
            {
                char buffer[4096] = {0};
                int client_fd = _cgi_to_client[fd];
                size_t bytes_read = read(fd, buffer, sizeof(buffer));
				const std::map<int, Client>::iterator it = _clients.find(client_fd);
				if (it == _clients.end())
					return;

				Client& client = it->second;
                if (bytes_read > 0)
				{
					std::string tmp(buffer,bytes_read);
					if (!client.req.foundHeader)
					{
						client.req.str += std::string(buffer, bytes_read);
						size_t pos = client.req.str.find("\r\n\r\n");
                        size_t sep_size = 4;
                        if (pos == std::string::npos)
                        {
                            pos = client.req.str.find("\n\n");
                            sep_size = 2;
                        }
						if (pos != std::string::npos)
						{
							client.req.foundHeader = 1;
							std::string header;
							header = client.res.handleResponse(client.req, 200);
							send_data(client_fd, &header[0], header.size());
							client.req.dataCgi.insert(client.req.dataCgi.end(), client.req.str.begin() + pos + sep_size, client.req.str.end());
						}
					}
					else
					{
						std::string tmp = std::string(buffer, bytes_read);
						client.req.dataCgi.insert(client.req.dataCgi.end(),tmp.begin(),tmp.end());
					}

					if (client.req.dataCgi.size() >= 0x8000)
					{
						send_data(client_fd, "8000\r\n", 6);
						send_data(client_fd, &client.req.dataCgi[0], 0x8000);
						send_data(client_fd, "\r\n", 2);
						client.req.dataCgi.erase(client.req.dataCgi.begin(),client.req.dataCgi.begin() + 0x8000);
					}
				}
                else
                {
					if (bytes_read == 0)
					{
						std::stringstream ss;
						ss << std::hex << client.req.dataCgi.size();
						if (!client.req.foundHeader)
						{
							std::string header;
							header = client.res.handleResponse(client.req, 500);
							send(client_fd,&header[0],header.size(),0);
						}
						else
						{
							if (client.req.dataCgi.size() != 0)
							{
								std::string hexa = ss.str();
								hexa += "\r\n";
								send(client_fd,&hexa[0],hexa.size(),0);
								send(client_fd, &client.req.dataCgi[0], client.req.dataCgi.size(), 0);
								send(client_fd,"\r\n",2,0);
							}
							send(client_fd,"0",1,0);
							send(client_fd,"\r\n",2,0);
							send(client_fd,"\r\n",2,0);
						}
						remove_from_epoll(fd);
						close(fd);
						_fd_types.erase(fd);
						int pid = _map_cgi_pid[fd];
						int status;
						waitpid(pid, &status, 0);
						_map_cgi_pid.erase(fd);
						_cgi_to_client.erase(fd);
						modify_epoll(client_fd, EPOLLOUT | EPOLLET);
					}
                }
                continue;
            }
    
            if (_fd_types.find(fd) == _fd_types.end())
            {
                std::cerr << "Unknown fd type for fd " << fd << std::endl;
                continue;
            }
            if (_fd_types[fd] == FD_LISTENER)
                handle_new_connection(fd);
            else if (_fd_types[fd] == FD_CLIENT)
            {
                if (events[i].events & (EPOLLHUP | EPOLLERR))
                    handle_client_disconnect(fd);
                else if (events[i].events & EPOLLIN)
                    handle_client_read(fd);
                else if (events[i].events & EPOLLOUT)
                    handle_client_write(fd);
            }
        }
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); )
        {
            if (it->second.is_timed_out() || it->second.get_status())
            {
 std::cout << "Client " << it->first << " timed out or closed, disconnecting" << std::endl;
                const int fd = it->first;
                remove_from_epoll(fd);
                _fd_types.erase(fd);
                it->second.close();
                const std::map<int, Client>::iterator to_erase = it;
                ++it;
                _clients.erase(to_erase);
            }
            else
                ++it;
        }


    }
    stop();
}

void Engine::stopFork()
{
    _is_running = false;

    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.req.tmpName.size() != 0)
		{
			close(it->second.req.fd);
		}		
        it->second.close();
	}
    for (std::map<int, Listener>::iterator it = _listeners.begin(); it != _listeners.end(); ++it)
        it->second.close_socket();
}

void Engine::stop()
{
    _is_running = false;

    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.req.tmpName.size() != 0)
		{
			close(it->second.req.fd);
			unlink(it->second.req.tmpName.c_str());
		}		
        it->second.close();
	}
    _clients.clear();
    for (std::map<int, Listener>::iterator it = _listeners.begin(); it != _listeners.end(); ++it)
        it->second.close_socket();
    _listeners.clear();
    _fd_types.clear();
}

