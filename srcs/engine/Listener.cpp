//
// Created by tjooris on 2/16/26.
//

#include "Listener.hpp"
#include <unistd.h>

Listener::Listener() : _fd(-1), _port(0), _host(0) {}

Listener::Listener(int host, int port) : _fd(-1), _port(port), _host(host) {}

Listener::~Listener() {
	close_socket();
}

bool Listener::create_socket() {
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	return _fd != -1;
}

bool Listener::bind_socket()
{

}
