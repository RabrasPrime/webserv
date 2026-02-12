#ifndef SERVER_HPP
# define SERVER_HPP

#include "Location.hpp"
#include "Config.hpp"

class Server : public Config
{
	public:
		Server(){}
		~Server(){}

	private:
		int								_host;
		int								_port;
		std::string						_server_name;
		std::map<std::string, Location>	_locations;

	public:
		int										get_host() const;
		int										get_port() const;
		const std::string						get_server_name() const;
		const std::map<std::string, Location>	get_locations() const;
};

#endif