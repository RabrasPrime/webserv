//
// Created by tjooris on 2/16/26.
//

#ifndef WEBSERV_CLIENT_HPP
#define WEBSERV_CLIENT_HPP

#include "server/Server.hpp"

class Server;

class Client
{
	private:
		int	_fd;
		Server	*_server;
	public:
		Client(int fd, Server *server);
		~Client();

		int get_fd() const;
		Server* get_server() const;

};


#endif //WEBSERV_CLIENT_HPP