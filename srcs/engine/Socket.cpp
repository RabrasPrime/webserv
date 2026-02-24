#include "Server.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>


int	create_socket(int host, int port)
{
	int								socket_fd;
	struct sockaddr_storage			socket_addr;


}