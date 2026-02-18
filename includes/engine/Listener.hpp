#pragma once

#include <vector>
#include <string>

class Server;

class Listener
{
private:
	int _fd;
	int _port;
	int _host;
	std::vector<Server*> _servers;

	static const int BACKLOG = 128;

public:
	Listener();
	Listener(int host, int port);
	~Listener();

	bool create_socket();
	bool bind_socket();
	bool listen_socket();
	void close_socket();

	int accept_connection();

	void add_server(Server* server);
	void remove_server(Server* server);
	Server* match_server(const std::string& host_header) const;

	int get_fd() const;
	int get_port() const;
	int get_host() const;
	const std::vector<Server*>& get_servers() const;

	bool is_valid() const;
	void set_non_blocking();
};
