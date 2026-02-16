//
// Created by tjooris on 2/16/26.
//

#ifndef WEBSERV_CLIENT_HPP
#define WEBSERV_CLIENT_HPP

#include "Server.hpp"
#include <string>

class Server;

class Client
{
	private:
		int	_fd;
		Server	*_server;

		std::string	_read_buffer;
		std::string	_write_buffer;
	public:
		Client();
		Client(int fd, Server *server);
		~Client();

		int get_fd() const;
		Server* get_server() const;

		std::string& get_read_buffer();
		const std::string& get_read_buffer() const;

		std::string& get_write_buffer();
		const std::string& get_write_buffer() const;

		char& operator[](size_t index);
		const char& operator[](size_t index) const;

		bool is_valid() const;
};


#endif //WEBSERV_CLIENT_HPP