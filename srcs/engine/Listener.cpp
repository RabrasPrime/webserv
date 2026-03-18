//
// Created by tjooris on 2/16/26.
//

#include "Listener.hpp"
#include "Server.hpp"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>

const	int	Listener::BACKLOG;

Listener::Listener() : _fd(-1), _port(0), _host(0) {}

Listener::Listener(int host, int port, struct sockaddr_storage addrs) : _fd(-1), _port(port), _host(host), addr(addrs) {}

Listener::~Listener() {

}



bool Listener::create_socket() {
	if (addr.ss_family == AF_INET6)
	{
		_fd = socket(AF_INET6, SOCK_STREAM, 0);
	}
	else
	{
		_fd = socket(AF_INET, SOCK_STREAM, 0);
	}
	if (_fd < 0)
	{
		std::cerr << "Error creating socket" << std::endl;
		return false;
	}
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		std::cerr << "Error setting SO_REUSEADDR" << std::endl;
		close_socket();
		return false;
	}

	set_non_blocking();
	return true;
}

bool Listener::bind_socket() const
{

	if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
		if (bind(_fd, (struct sockaddr *)addr6, sizeof(struct sockaddr_in6)) < 0)
		{
			std::cerr << "Error binding IPv6 socket on port " << _port << std::endl;
			std::cerr << "Erreur lors du bind: " << strerror(errno) << std::endl;
			return false;
		}
	}
	else
	{
		struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
		if (bind(_fd, (struct sockaddr *)addr4, sizeof(struct sockaddr_in)) < 0)
		{
			std::cerr << "Error binding IPv4 socket on port " << _port << std::endl;
			std::cerr << "Erreur lors du bind: " << strerror(errno) << std::endl;
			return false;
		}
	}

	return true;
}

bool Listener::listen_socket() const
{
	if (listen(_fd, BACKLOG) < 0)
	{
		std::cerr << "Error listening on socket" << std::endl;
		return false;
	}
	return true;
}

void Listener::close_socket()
{
	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
	}
}

int	Listener::get_fd() const
{
	return _fd;
}

int Listener::get_port() const
{
	return _port;
}

int Listener::get_host() const
{
	return _host;
}

const std::vector<Server*>& Listener::get_servers() const
{
	return _servers;
}

bool Listener::is_valid() const
{
	return _fd != -1;
}

void Listener::set_non_blocking() const
{
	const int flags = fcntl(_fd, F_GETFL, 0);
	if (flags < 0)
	{
		std::cerr << "Error getting socket flags" << std::endl;
		return;
	}
	if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		std::cerr << "Error setting socket to non-blocking" << std::endl;
	}
}

int Listener::accept_connection() const
{
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(client_addr);

	const int client_fd = accept(_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
	if (client_fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			std::cerr << "Error accepting connection" << std::endl;
		}
		return -1;
	}
	char client_ip[INET6_ADDRSTRLEN];
	if (client_addr.ss_family == AF_INET)
	{
		const struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in*>(&client_addr);
		inet_ntop(AF_INET, &addr->sin_addr, client_ip, sizeof(client_ip));
	}
	else if (client_addr.ss_family == AF_INET6)
	{
		const struct sockaddr_in6 *addr = reinterpret_cast<struct sockaddr_in6*>(&client_addr);
		inet_ntop(AF_INET6, &addr->sin6_addr, client_ip, sizeof(client_ip));
	}
	const int flags = fcntl(client_fd, F_GETFL, 0);
	fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

	return client_fd;
}

void Listener::add_server(Server* server)
{
	_servers.push_back(server);
}

void Listener::remove_server(Server* server)
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers[i] == server)
		{
			_servers.erase(_servers.begin() + i);
			return;
		}
	}
}

Server* Listener::match_server(const std::string& host_header) const
{
	if (host_header.empty() || _servers.size() == 1)
	{
		return _servers.empty() ? NULL : _servers[0];
	}

	for (size_t i = 0; i < _servers.size(); i++)
	{
	const std::vector<std::string>& names = _servers[i]->get_server_name();
	if (std::find(names.begin(), names.end(), host_header) != names.end())
		{
			return _servers[i];
		}
	}
	return _servers[0];
}
