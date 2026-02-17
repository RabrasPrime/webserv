//
// Created by tjooris on 2/16/26.
//

#ifndef WEBSERV_CLIENT_HPP
#define WEBSERV_CLIENT_HPP

#include <stdio.h>

#include "Server.hpp"
#include <string>
#include <sys/time.h>

class Server;

class Client
{
	private:
		int	_fd;
		Server	*_server;

		std::string	_read_buffer;
		std::string	_write_buffer;

		struct timeval	_last_active_time;

		enum State
		{
			READING,
			WRITING,
			PROCESSING,
			CLOSED
		};
		State _state;
	public:
		Client();
		Client(int fd, Server *server);
		~Client();

		ssize_t	read_from_socket();
		ssize_t	write_to_socket();

		void	update();
		bool	is_timed_out() const;
		bool	should_continue() const;

		int get_fd() const;
		Server* get_server() const;
		void	set_server(Server* server);

		std::string& get_read_buffer();
		const std::string& get_read_buffer() const;

		std::string& get_write_buffer();
		const std::string& get_write_buffer() const;

		State	get_state();
		void	set_state(State new_state);

		char& operator[](size_t index);
		const char& operator[](size_t index) const;

		bool is_valid() const;
		void close();
		void set_non_blocking();
};


#endif //WEBSERV_CLIENT_HPP