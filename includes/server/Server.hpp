#ifndef SERVER_HPP
# define SERVER_HPP

#include "Location.hpp"
#include "Config.hpp"

class Server : public Config
{
	public:
		Server();
		~Server();

		int fill_server_config(std::ifstream& file, std::string& line);
		friend	std::ostream& operator<<(std::ostream& out, const Server& serv);

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

		int		set_listen(const std::string& value);
		int		set_server_name(const std::string& value);
};


#endif