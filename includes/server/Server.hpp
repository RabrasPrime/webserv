#ifndef SERVER_HPP
# define SERVER_HPP

#include "Location.hpp"
#include "Config.hpp"


#include <netinet/in.h>
#include <arpa/inet.h>

class Location;

struct User
{
	std::string UserName;
	std::string email;
	std::string password;
	int victory;
	int defeat;
};

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
		std::vector<std::string>				_server_name;
		std::map<std::string, Location>			_locations;
		std::map<std::string, User> _userData;

	public:
		const std::vector<struct sockaddr_storage>	get_addr() const;
		const std::vector<std::string>&				get_server_name() const;
		const std::map<std::string, Location>		get_locations() const;
		User&   getUser(std::string &username);
		bool userExist(const std::string &username);
		const std::map<std::string, Location>& get_locations() const;

		int		set_listen(const std::string& value);
		int		set_server_name(const std::string& value);
		void	setNewUser(User &newUser);
		
};


#endif
