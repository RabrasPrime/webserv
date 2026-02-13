#include "Server.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>


Server::Server()
: _host(0)
, _port(80)
{

}
Server::~Server()
{

}

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
std::ostream& operator<<(std::ostream& out, const Server& serv)
{
	out << "Host : "	<< ((serv._host >> 24) & 255)
						<< "."
						<< ((serv._host >> 16) & 255)
						<< "."
						<< ((serv._host >> 8) & 255)
						<< "."
						<< (serv._host & 255)
						<< std::endl;
	out << "Port : " << serv._port << std::endl;
	out << "Server Name : " << serv._server_name << std::endl;
	out << "Client Max Body Size : " << serv._client_max_body_size << std::endl;
	out << "Error Pages : " << std::endl;
	for (std::map<int, std::string>::const_iterator it = serv._error_pages.begin(); it != serv._error_pages.end();it++)
		out << "\t" << it->first << " " << it->second << std::endl;
	return (out);
}

int is_valid_octet_addr(int value)
{
	if (value < 0 || value > 255)
		return (0);
	return (1);
}


int		Server::set_listen(const std::string& value)
{
	char extra;
	size_t pos1 = value.find(':');
	size_t pos2 = value.find('.');

	if (pos1 != std::string::npos)
	{
		// adress et port
		std::stringstream ss(value);
		int addr_part1;
		int addr_part2;
		int addr_part3;
		int addr_part4;
		int port;
		char c1;
		char c2;
		char c3;
		char c4;
		if (ss >> addr_part1 >> c1 >> addr_part2 >> c2 >> addr_part3 >> c3 >> addr_part4 >> c4 >> port)
		{
			if (ss >> extra
				|| !is_valid_octet_addr(addr_part1)
				|| !is_valid_octet_addr(addr_part2)
				|| !is_valid_octet_addr(addr_part3)
				|| !is_valid_octet_addr(addr_part4)
				|| port <= 0
				|| c1 != '.' || c2 != '.' || c3 != '.')
				return (1);
			_host = (addr_part1 << 24) + (addr_part2 << 16) + (addr_part3 << 8) + addr_part4;
			_port = port;
		}
		else
			return (1);
	}
	else if (pos2 != std::string::npos)
	{
		// adresse
		std::stringstream ss(value);
		int addr_part1;
		int addr_part2;
		int addr_part3;
		int addr_part4;
		char c1;
		char c2;
		char c3;
		if (ss >> addr_part1 >> c1 >> addr_part2 >> c2 >> addr_part3 >> c3 >> addr_part4)
		{
			if (ss >> extra
				|| !is_valid_octet_addr(addr_part1)
				|| !is_valid_octet_addr(addr_part2)
				|| !is_valid_octet_addr(addr_part3)
				|| !is_valid_octet_addr(addr_part4)
				|| c1 != '.' || c2 != '.' || c3 != '.')
				return (1);
			_host = (addr_part1 << 24) + (addr_part2 << 16) + (addr_part3 << 8) + addr_part4;
		}
		else
			return (1);
	}
	else
	{
		// port
		std::stringstream ss(value);
		if (ss >> _port)
		{
			if (ss >> extra || _port <= 0)
				return (1);
		}
		else
			return (1);
	}
	return (0);
}
int		Server::set_server_name(const std::string& value)
{
	if (value.size() <= 0)
		return (1);
	_server_name = value;
	return (0);
}


int Server::fill_server_config(std::ifstream& file)
{
	std::string line;

	while (std::getline(file, line))
	{
		if (count_char(line, '\t') < 1 && line.size() > 0)
		{
			std::cout << *this << std::endl;
			std::cout << "end serv scope" << std::endl;
			return (1);
		}
		size_t i = 0;
		while (std::isspace(line[i]))
			i++;
		std::string key;
		while (!std::isspace(line[i]) && i < line.size())
		{
			key += line[i];
			i++;
		}
		while (std::isspace(line[i]))
			i++;
		std::string value(&line[i]);
		if (key == "listen")
			set_listen(value) ? print_warning("Warning","Invalid value on listen !") : "";
		if (key == "server_name")
			set_server_name(value) ? print_warning("Warning","Invalid value on server_name !"): "";
		if (key == "root")
			set_root(value) ? print_warning("Warning","Invalid value on root !"): "";
		if (key == "client_max_body_size")
			set_client_max_body_size(value) ? print_warning("Warning","Invalid value on client_max_body_size !"): "";
		if (key == "error_pages")
			set_error_pages(value) ? print_warning("Warning","Invalid value on error_pages !"): "";
		if (key == "methods")
	}
	if (file.eof())
	{
		std::cout << *this << std::endl;
		std::cout << "end serv scope" << std::endl;
	}
	return (0);
}