#ifndef SERVER_HPP
# define SERVER_HPP

#include "Location.hpp"
#include "Config.hpp"


#include <netinet/in.h>
#include <arpa/inet.h>

class Location;

class Server : public Config
{
	public:
		Server();
		~Server();

		int fill_server_config(std::ifstream& file, std::string& line);
		friend	std::ostream& operator<<(std::ostream& out, const Server& serv);
		void fill_locations();

	private:
		std::vector<struct sockaddr_storage>	_addr;
		std::string								_server_name;
		std::map<std::string, Location>			_locations;

	public:
		const std::vector<struct sockaddr_storage>	get_addr() const;
		const std::string							get_server_name() const;
		const std::map<std::string, Location>		get_locations() const;

		int		set_listen(const std::string& value);
		int		set_server_name(const std::string& value);
};


#endif