#include "Location.hpp"
#include "Utils.hpp"
#include <fstream>

const std::string			Location::get_path() const
{
	return (_path);
}

int Location::fill_location_config(std::ifstream& file, std::string& line, int indent)
{
	while (std::getline(file, line))
	{
		if (count_char(line, '\t') < indent + 1)
		{
			//print_location();
		}
	}
	return (0);
}