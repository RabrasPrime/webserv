#include <map>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>


int main()
{
	int								_host;
	int								_port;
	std::string						_server_name;
	std::map<int, bool>				_is_socket_listening;
	int								_epoll_fd;




	return (0);
}