#include "Server.hpp"
#include "Utils.hpp"
#include "Color.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <netdb.h>
#include <iomanip>

Server::Server()
{

}
Server::~Server()
{

}

const std::vector<std::string>&			Server::get_server_name() const
{
	return (_server_name);
}
const std::map<std::string, Location>	Server::get_locations() const
{
	return (_locations);
}
const std::vector<struct sockaddr_storage>	Server::get_addr() const
{
	return (_addr);
}
void print_ipv4(const struct sockaddr_in* addr)
{
	unsigned char* str = (unsigned char*)&(addr->sin_addr.s_addr);
	std::cout << (int)str[0] << "." << (int)str[1] << "." << (int)str[2] << "." << (int)str[3];
}
void print_ipv6(const struct sockaddr_in6* addr)
{
	const unsigned char* p = addr->sin6_addr.s6_addr;

    for (int i = 0; i < 16; i += 2) {
        std::cout << std::hex << std::setw(1) 
                  << ((p[i] << 8) | p[i+1]);
        if (i < 14) {
            std::cout << ":";
        }
    }
    std::cout << std::dec;
}
std::ostream& operator<<(std::ostream& out, const Server& serv)
{
	out << "Addr : " << std::endl;
	for (std::vector<struct sockaddr_storage>::const_iterator it = serv._addr.begin();it != serv._addr.end(); it++)
	{
		int port;
		if (it->ss_family == AF_INET) {
			const struct sockaddr_in* addr4 = (const struct sockaddr_in*)&(*it);
			port = ntohs(addr4->sin_port);
			std::cout << "\tIPv4: ";
			print_ipv4(addr4);
			std::cout << ":" << port << std::endl;
		} 
		else if (it->ss_family == AF_INET6) {
			const struct sockaddr_in6* addr6 = (const struct sockaddr_in6*)&(*it);
			// inet_ntop(AF_INET6, &(addr6->sin6_addr), ip_str, INET6_ADDRSTRLEN);
			port = ntohs(addr6->sin6_port);
			std::cout << "\tIPv6: [";
			print_ipv6(addr6);
			std::cout << "]:" << port << std::endl;
		}
	}
	out << BLUE << "Server Name : ";// << PURPLE << serv._server_name << std::endl;
	for (std::vector<std::string>::const_iterator it = serv._server_name.begin(); it != serv._server_name.end();it++)
		std::cout << "\t" << *it << std::endl;
	out << static_cast<Config>(serv);

	for (std::map<std::string, Location>::const_iterator it = serv._locations.begin();it != serv._locations.end();it++)
	{
		out << it->second;
	}
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
	struct sockaddr_storage storage;
	std::memset(&storage, 0, sizeof(storage)); 

	if (value.find('.') != std::string::npos)
	{
		//ipv4
		size_t pos = value.find(':');
		if (pos != std::string::npos)
		{
			std::string host(value.substr(0, pos));
			std::stringstream ss(&value[pos + 1]);
			std::string port;
			if (!(ss >> port))
				return (1);
			char extra;
			if (ss >> extra)
				return (1);

			struct addrinfo hints,*res;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_NUMERICHOST;
			if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res))
				return (1);
			memcpy(&storage, res->ai_addr, res->ai_addrlen);
		}
		else
		{
			std::string host(value);
			struct addrinfo hints,*res;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_NUMERICHOST;
			if (getaddrinfo(host.c_str(), "80", &hints, &res))
				return (1);
			memcpy(&storage, res->ai_addr, res->ai_addrlen);
		}		
	}
	else if (value.find(':') != std::string::npos)
	{
		//ipv6
		size_t pos = value.find(']');
		if (pos != std::string::npos)
		{
			std::string host(value.substr(1, pos - 1));
			if (value[pos + 1] != ':')
				return (1);
			std::stringstream ss(&value[pos + 2]);
			std::string port;
			if (!(ss >> port))
				return (1);
			char extra;
			if (ss >> extra)
				return (1);
			struct addrinfo hints,*res;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET6;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_NUMERICHOST;
			if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res))
				return (1);
			memcpy(&storage, res->ai_addr, res->ai_addrlen);
		}
		else
		{
			if (value.find(' ') != std::string::npos)
				return (1);
			struct addrinfo hints,*res;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET6;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_NUMERICHOST;
			if (getaddrinfo(value.c_str(), "80", &hints, &res))
				return (1);
			memcpy(&storage, res->ai_addr, res->ai_addrlen);
		}

	}
	else
	{
		//port
		std::string port;
		std::stringstream ss(value);
		if (!(ss >> port))
			return (1);
		char extra;
		if (ss >> extra)
			return (1);
		struct addrinfo hints,*res;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_NUMERICHOST;
		if (getaddrinfo("0.0.0.0", port.c_str(), &hints, &res))
			return (1);
		memcpy(&storage, res->ai_addr, res->ai_addrlen);
	}
	_addr.push_back(storage);
	return (0);
}
int		Server::set_server_name(const std::string& value)
{
	if (value.size() <= 0)
		return (1);
	std::stringstream ss(value);
	std::string str;
	while (ss >> str)
	{
		_server_name.push_back(str);
	}
	return (0);
}

int Server::fill_server_config(std::ifstream& file, std::string& line)
{
	int read = 0;
	while (read || std::getline(file, line))
	{
		read = 0;
		if (count_char(line, '\t') < 1 && line.size() > 0)
		{
			fill_locations();
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
			set_listen(value) ? print_warning("Warning","Invalid value on listen !", value) : "";
		else if (key == "server_name")
			set_server_name(value) ? print_warning("Warning","Invalid value on server_name !", value): "";
		else if (key == "locations")
		{
			Location loc;
			std::stringstream ss(value);
			std::string str;
			read = loc.fill_location_config(file, line, count_char(line, '\t'));
			while (ss >> str)
			{
				Location tmp;
				tmp = loc;
				tmp.set_path(str);
				this->_locations[str] = tmp;
			}
		}
		else
			fill_config(key, value);
	}
	if (file.eof())
	{
		fill_locations();
	}
	return (0);
}

void Server::fill_locations()
{
	for (std::map<std::string, Location>::iterator it = _locations.begin();it != _locations.end();it++)
	{
		it->second.heritage_from_server(*this);
	}
}