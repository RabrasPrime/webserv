#include "Location.hpp"
#include "Utils.hpp"
#include <fstream>

const std::string			Location::get_path() const
{
	return (_path);
}

void		Location::set_path(const std::string value)
{
	_path = value;
}

std::ostream& operator<<(std::ostream& out, const Location& loc)
{
	out << "Path : " << loc._path << std::endl;
	out << static_cast<Config>(loc);
	return (out);
}
const Location& Location::operator=(const Location& loc)
{
	_path = loc._path;
	_root = loc._root;
	_client_max_body_size = loc._client_max_body_size;
	_error_pages = loc._error_pages;
	_methods = loc._methods;
	_auto_index = loc._auto_index;
	_indexes = loc._indexes;
	_upload_path = loc._upload_path;
	_cgi_enabled = loc._cgi_enabled;
	_cgi_ext = loc._cgi_ext;
	_cgi_working_dir = loc._cgi_working_dir;
	_cgi_upload_path = loc._cgi_upload_path;
	_cgi_timeout = loc._cgi_timeout;
	return (*this);
}


int Location::fill_location_config(std::ifstream& file, std::string& line, int indent)
{
	while (std::getline(file, line))
	{
		if (count_char(line, '\t') < indent + 1)
		{
			// std::cout << *this << std::endl;
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
		fill_config(key, value);
	}
	return (0);
}