//
// Created by tjooris on 2/16/26.
//

#ifndef WEBSERV_ENGINE_HPP
#define WEBSERV_ENGINE_HPP

#include <map>
#include <string>

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

		bool	_is_running;
		static	const int	_max_events = 64;
	public:
		Engine();
		~Engine();

		void	init_listeners();
		void	setupt_epoll();

		void	run();
		void	stop();

		void	add_client(Client* client);
		void	remove_client(Client* client);

		void	add_server(Server* server);
		void	remove_server(Server* server);
		Server*	match_server(int host, int port);

		void	add_listener(Listener* listener);
		void	remove_listener(Listener* listener);

		bool	is_running();

		bool	create_socket(int host, int port);
		bool	bind_socket(int socket_fd, int host, int port);
		bool	listen_socket(int socket_fd);
		void	close_socket(int socket_fd);


};

#endif //WEBSERV_ENGINE_HPP