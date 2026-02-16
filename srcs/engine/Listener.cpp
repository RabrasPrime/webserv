//
// Created by tjooris on 2/16/26.
//

#include "Listener.hpp"

Listener::Listener(): fd(-1){}

Listener::Listener(Server* server): fd(-1)
{
	_servers.push_back(server);
}

Listener::Listener(int fd): fd(fd){}

Listener::Listener(Server* server, int fd): fd(fd)
{
	_servers.push_back(server);
}

Listener::~Listener(){}

int Listener::get_fd() const
{
	return (fd);
}

void Listener::add_server(Server* server)
{
	_servers.push_back(server);
}

void Listener::remove_server(Server* server)
{
	std::vector<Server*>::iterator it = std::find(_servers.begin(), _servers.end(), server);
	if (it != _servers.end())
		_servers.erase(it);
}

void Listener::add_fd(int fd)
{
	this->fd = fd;
}

std::vector<Server*> &Listener::get_servers() const
{
	return ((std::vector<Server*>&)_servers);
}
