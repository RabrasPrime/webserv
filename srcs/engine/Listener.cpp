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

const	int	Listener::BACKLOG;

Listener::Listener() : _fd(-1), _port(0), _host(0) {}

Listener::Listener(int host, int port) : _fd(-1), _port(port), _host(host) {}

Listener::~Listener() {
	close_socket();
}



bool Listener::create_socket() {
	_fd = socket(AF_INET6, SOCK_STREAM, 0);
	if (_fd < 0) {
		std::cerr << "Error creating IPv6 socket, falling back to IPv4" << std::endl;
		_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (_fd < 0) {
			std::cerr << "Error creating socket" << std::endl;
			return false;
		}
	} else {
		int no = 0;
		if (setsockopt(_fd, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) < 0) {
			std::cerr << "Warning: Could not disable IPV6_V6ONLY, IPv4 may not work" << std::endl;
		}
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

bool Listener::bind_socket()
{
	struct sockaddr_storage storage;

	std::memset(&storage, 0, sizeof(storage));
	int domain;
	socklen_t len = sizeof(domain);
	if (getsockopt(_fd, SOL_SOCKET, SO_DOMAIN, &domain, &len) < 0) {
		std::cerr << "Error getting socket domain" << std::endl;
		return false;
	}

	if (domain == AF_INET6)
	{
		struct sockaddr_in6 *addr6 = static_cast<struct sockaddr_in6*>(
			static_cast<void*>(&storage)
		);
		addr6->sin6_family = AF_INET6;
		addr6->sin6_port = htons(_port);

		if (_host == 0) {
			addr6->sin6_addr = in6addr_any;
		} else {
			addr6->sin6_addr = in6addr_any;
		}

		if (bind(_fd, reinterpret_cast<struct sockaddr*>(addr6), sizeof(*addr6)) < 0) {
			std::cerr << "Error binding IPv6 socket on port " << _port << std::endl;
			return false;
		}
	}
	else
	{
		struct sockaddr_in *addr4 = static_cast<struct sockaddr_in*>(
			static_cast<void*>(&storage)
		);
		addr4->sin_family = AF_INET;
		addr4->sin_port = htons(_port);
		addr4->sin_addr.s_addr = htonl(_host);

		if (bind(_fd, reinterpret_cast<struct sockaddr*>(addr4), sizeof(*addr4)) < 0)
		{
			std::cerr << "Error binding IPv4 socket on port " << _port << std::endl;
			return false;
		}
	}

	return true;
}

bool Listener::listen_socket()
{
	if (listen(_fd, BACKLOG) < 0)
	{
		std::cerr << "Error listening on socket" << std::endl;
		return false;
	}
	std::cout << "Listening on port " << _port << std::endl;
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

void Listener::set_non_blocking()
{
	int flags = fcntl(_fd, F_GETFL, 0);
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

int Listener::accept_connection()
{
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(_fd, (struct sockaddr*)&client_addr, &client_len);
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
		struct sockaddr_in *addr = (struct sockaddr_in*)&client_addr;
		inet_ntop(AF_INET, &addr->sin_addr, client_ip, sizeof(client_ip));
		std::cout << "New IPv4 connection from " << client_ip << std::endl;
	}
	else if (client_addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6 *addr = (struct sockaddr_in6*)&client_addr;
		inet_ntop(AF_INET6, &addr->sin6_addr, client_ip, sizeof(client_ip));
		std::cout << "New IPv6 connection from " << client_ip << std::endl;
	}
	int flags = fcntl(client_fd, F_GETFL, 0);
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
		if (_servers[i]->get_server_name() == host_header)
		{
			return _servers[i];
		}
	}
	return _servers[0];
}