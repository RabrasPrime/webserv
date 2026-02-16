//
// Created by tjooris on 2/16/26.
//

#ifndef WEBSERV_LISTENER_HPP
#define WEBSERV_LISTENER_HPP

#include "Server.hpp"
#include <vector>

class Server;

class Listener
{
	private:
		int	fd;
		std::vector<Server*> _servers;
	public:
		Listener();
		Listener(Server* server);
		Listener(int fd);
		Listener(Server* server, int fd);
		~Listener();

		int	get_fd() const;
		void	add_server(Server* server);
		void	remove_server(Server* server);
		void	add_fd(int fd);
		std::vector<Server*> &get_servers() const;
};


#endif //WEBSERV_LISTENER_HPP