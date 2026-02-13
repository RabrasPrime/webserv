#ifndef LOCATION_HPP
# define LOCATION_HPP

#include <iostream>
#include <vector>
#include <map>
#include "Config.hpp"


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