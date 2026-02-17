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

		int	_port;
		int	_host;
		static const int	_backlog = 128;
	public:
		Listener();
		Listener(Server* server);
		Listener(int fd);
		Listener(Server* server, int fd);
		~Listener();

		void	add_server(Server* server);
		void	remove_server(Server* server);
		Server*	match_server(int host, int port) const;
		void	add_fd(int fd);

		bool	create_socket(int host, int port);
		bool	bind_socket(int host, int port);
		bool	listen_socket();
		void	close_socket();

		int	accept_connection();

		int	get_fd() const;
		int	get_port() const;
		int	get_host() const;
		const std::vector<Server*>& get_servers() const;

		bool	is_valid() const;
		void	set_non_blocking();
		void	set_non_blocking(int fd);
};


#endif //WEBSERV_LISTENER_HPP