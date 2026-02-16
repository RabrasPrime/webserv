//
// Created by tjooris on 2/16/26.
//

#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

Client::Client(): _fd(-1), _server(NULL){}

Client::Client(int fd, Server *server): _fd(fd), _server(server){}

Client::~Client(){}

int Client::get_fd() const
{
	return (_fd);
}

Server* Client::get_server() const
{
	return (_server);
}

std::string& Client::get_read_buffer()
{
	return (_read_buffer);
}

const std::string& Client::get_read_buffer() const
{
	return (_read_buffer);
}

std::string& Client::get_write_buffer()
{
	return (_write_buffer);
}

const std::string& Client::get_write_buffer() const
{
	return (_write_buffer);
}

char& Client::operator[](size_t index)
{
	return (_read_buffer[index]);
}

const char& Client::operator[](size_t index) const
{
	return (_read_buffer[index]);
}

bool Client::is_valid() const
{
	return (_fd != -1);
}