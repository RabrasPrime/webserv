//
// Created by tjooris on 2/16/26.
//

#include "Listener.hpp"
#include <unistd.h>
#include <iostream>
#include <cstring>

const	int	Listener::BACKLOG;

Listener::Listener() : _fd(-1), _port(0), _host(0) {}

Listener::Listener(int host, int port) : _fd(-1), _port(port), _host(host) {}

Listener::~Listener() {
	close_socket();
}

bool Listener::create_socket()
{
	_fd = socket(AF_INET6, SOCK_STREAM, 0);
	if (_fd < 0)
	{
		std::cerr << "Error creating socket" << std::endl;
		return false;
	}
	int opt = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	int no = 0;
	if (setsockopt(_fd, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) < 0)
	{
		std::cerr << "Warning: Could not disable IPV6_V6ONLY" << std::endl;
	}
	set_non_blocking();
	return true;
}

bool Listener::bind_socket()
{
	struct sockaddr_in6 addr;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(_port);
	addr.sin6_addr = in6addr_any;

	if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		std::cerr << "Error binding socket on port " << _port << std::endl;
		return false;
	}
	return true;
}

bool Listener::listen_socket()
{
	if (listen(_fd, 10) < 0)
	{
		std::cerr << "Error listening on socket" << std::endl;
		return false;
	}
	std::cout << "Listening on socket" << std::endl;
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

