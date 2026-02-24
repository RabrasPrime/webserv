//
// Created by tjooris on 2/16/26.
//

#include "Client.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <iostream>

#include "Color.hpp"

Client::Client() : _fd(-1), _server(NULL), _status(READING)
{
	std::cout << LIME << "Open1 Client" << RESET << std::endl;
	std::memset(&_last_active_time, 0, sizeof(_last_active_time));
	gettimeofday(&_last_active_time, NULL);
}

Client::Client(int fd, Server *server) : _fd(fd), _server(server), _status(READING)
{
	std::cout << GREEN << "Open1bis	 Client" << RESET << std::endl;
	std::memset(&_last_active_time, 0, sizeof(_last_active_time));
	gettimeofday(&_last_active_time, NULL);
	set_non_blocking();
}

Client::~Client()
{
	// std::cout << RED << "Close Client" << RESET << std::endl;
	// close();
}

ssize_t Client::read_from_socket()
{
	char buffer[4096];
	ssize_t bytes_read = recv(_fd, buffer, sizeof(buffer), 0);

	if (bytes_read > 0)
	{
		_read_buffer.append(buffer, bytes_read);
		update();
	}
	else if (bytes_read == 0)
	{
		std::cout << "Client closed connection (fd " << _fd << ")" << std::endl;
		return 0;
	}
	else
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			std::cerr << "Error reading from socket fd " << _fd << ": "
			          << std::strerror(errno) << std::endl;
			return -1;
		}
		return 0;
	}

	return bytes_read;
}

ssize_t Client::write_to_socket()
{
	if (_write_buffer.empty())
		return 0;

	ssize_t bytes_sent = send(_fd, _write_buffer.c_str(), _write_buffer.size(), MSG_NOSIGNAL);

	if (bytes_sent > 0)
	{
		_write_buffer.erase(0, bytes_sent);
		update();
	}
	else if (bytes_sent < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			std::cerr << "Error writing to socket fd " << _fd << ": "
			          << std::strerror(errno) << std::endl;
			return -1;
		}
		return 0;
	}

	return bytes_sent;
}

void Client::update()
{
	gettimeofday(&_last_active_time, NULL);
}

bool Client::is_timed_out() const
{
	struct timeval now;
	gettimeofday(&now, NULL);

	const long TIMEOUT_SECONDS = 60;
	long elapsed = now.tv_sec - _last_active_time.tv_sec;

	return elapsed > TIMEOUT_SECONDS;
}

bool Client::should_continue() const
{
	return _status != CLOSED && !is_timed_out() && _fd >= 0;
}

int Client::get_fd() const
{
	return _fd;
}

Server* Client::get_server() const
{
	return _server;
}

void Client::set_server(Server* server)
{
	_server = server;
}

std::string& Client::get_read_buffer()
{
	return _read_buffer;
}

const std::string& Client::get_read_buffer() const
{
	return _read_buffer;
}

std::string& Client::get_write_buffer()
{
	return _write_buffer;
}

const std::string& Client::get_write_buffer() const
{
	return _write_buffer;
}

Client::Status Client::get_status() const
{
	return _status;
}

void Client::set_status(Status new_status)
{
	_status = new_status;
}

char& Client::operator[](size_t index)
{
	return _read_buffer[index];
}

const char& Client::operator[](size_t index) const
{
	return _read_buffer[index];
}

bool Client::is_valid() const
{
	return _fd >= 0 && _status != CLOSED;
}

void Client::close()
{
	if (_fd >= 0)
	{
		::close(_fd);
		_fd = -1;
	}
	_status = CLOSED;
	_read_buffer.clear();
	_write_buffer.clear();
}

void Client::set_non_blocking()
{
	if (_fd < 0)
		return;

	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags < 0)
	{
		std::cerr << "Error getting socket flags for fd " << _fd << std::endl;
		return;
	}

	if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		std::cerr << "Error setting non-blocking mode for fd " << _fd << std::endl;
	}
}