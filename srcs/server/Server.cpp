#include "Server.hpp"

int								Server::get_host() const
{
	return (_host);
}
int								Server::get_port() const
{
	return (_port);
}
const std::string						Server::get_server_name() const
{
	return (_server_name);
}
const std::map<std::string, Location>	Server::get_locations() const
{
	return (_locations);
}