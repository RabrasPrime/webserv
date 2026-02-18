#ifndef LOCATION_HPP
# define LOCATION_HPP

#include <iostream>
#include <vector>
#include <map>
#include "Config.hpp"
#include "Server.hpp"

class Server;

class Location : public Config
{
	public:
		Location(){}
		~Location(){}

		int		fill_location_config(std::ifstream& file, std::string& line, int indent);
		friend	std::ostream& operator<<(std::ostream& out, const Location& loc);
		const Location& operator=(const Location& loc);
		void heritage_from_server(const Server& serv);

	private:
		std::string			_path;

	public:
		// GET
		const std::string		get_path() const;

		// SET
		void		set_path(const std::string value);
};

#endif