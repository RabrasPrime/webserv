//
// Created by tjooris on 2/16/26.
//

#include "Listener.hpp"
#include <unistd.h>

Listener::Listener() : _fd(-1), _port(0), _host(0) {}

Listener::Listener(Server* server) : _fd(-1), _port(0), _host(0)
{
	add_server(server);
}

Listener::Listener(int fd) : _fd(fd), _port(0), _host(0) {}

Listener::~Listener() {
	if (_fd != -1) {
		close(_fd);
	}
}

