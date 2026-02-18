//
// Created by tjooris on 2/16/26.
//

#include "Client.hpp"
#include "Server.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

Client::Client(): _fd(-1), _server(NULL), _state(READING)
{
	gettimeofday(&_last_active_time, NULL);
}

Client::Client(int fd, Server *server): _fd(fd), _server(server), _state(READING)
{
	gettimeofday(&_last_active_time, NULL);
}

Client::~Client()
{
	if (_fd != -1)
		close(_fd);
}
