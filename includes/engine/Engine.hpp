#pragma once

#include <map>
#include <vector>
#include <sys/epoll.h>
#include "Server.hpp"
#include "Listener.hpp"
#include "Client.hpp"

class Engine {
	private:
		int _epoll_fd;
		std::vector<Server> _servers_config;

		std::map<int, Client> _clients;
		std::map<int, Listener> _listeners;

		enum FdType
		{
			FD_LISTENER,
			FD_CLIENT
		};
		std::map<int, FdType> _fd_types;

		bool _is_running;
		static const int MAX_EVENTS = 64;

	public:
		Engine();
		~Engine();

		void init_listeners();
		void setup_epoll();

		void run();
		void stop();
		bool is_running() const;

		void handle_new_connection(int listener_fd);
		void handle_client_read(int client_fd);
		void handle_client_write(int client_fd);
		void handle_client_disconnect(int client_fd);

		void add_to_epoll(int fd, uint32_t events);
		void modify_epoll(int fd, uint32_t events);
		void remove_from_epoll(int fd);

		void add_server(const Server& server);
		Server* match_server(const std::string& host_header);
};

