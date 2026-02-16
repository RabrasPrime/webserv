//
// Created by tjooris on 2/16/26.
//

#ifndef WEBSERV_ENGINE_HPP
#define WEBSERV_ENGINE_HPP

#include <map>
#include "Server.hpp"
#include <vector>
#include <sys/epoll.h>
#include "Listener.hpp"
#include "Client.hpp"


class Client;
class Server;

class Engine
{
	private:
		int	_epoll_fd;
		std::vector<Server> _servers_config;

		std::map<int, Client> _clients;;
		std::map<int, Listener> _listening_engines;
	public:
		Engine();
		~Engine();
};


#endif //WEBSERV_ENGINE_HPP