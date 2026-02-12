#ifndef LOCATION_HPP
# define LOCATION_HPP

#include <iostream>
#include <vector>
#include <map>
#include "Config.hpp"

# define METHOD_GET 0b1
# define METHOD_POST 0b10
# define METHOD_DELETE 0b100

class Location : public Config
{
	public:
		Location(){}
		~Location(){}

	private:
		std::string			_path;

	public:
		const std::string	get_path() const;
};

#endif