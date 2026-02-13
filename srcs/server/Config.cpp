#include "Config.hpp"
#include <sstream>


Config::Config()
:_client_max_body_size(CLIENT_MAX_BODY_SIZE_DEFAULT)
,_methods(0)
,_auto_index(0)
,_cgi_enabled(0)
,_cgi_timeout(10)
{

}
Config::~Config()
{

}


const std::string							Config::get_root() const
{
	return (_root);
}
size_t										Config::get_client_max_body_size() const
{
	return (_client_max_body_size);
}
const std::map<int, std::string>			Config::get_error_pages() const
{
	return (_error_pages);
}
int											Config::get_methods() const
{
	return (_methods);
}
bool										Config::get_auto_index() const
{
	return (_auto_index);
}
const std::vector<std::string>				Config::get_indexes() const
{
	return (_indexes);
}
const std::string							Config::get_upload_store() const
{
	return (_upload_store);
}
bool										Config::get_cgi_enabled() const
{
	return (_cgi_enabled);
}
const std::map<std::string, std::string>	Config::get_cgi_ext() const
{
	return (_cgi_ext);
}
const std::string							Config::get_cgi_working_dir() const
{
	return (_cgi_working_dir);
}
const std::string							Config::get_cgi_upload_path() const
{
	return (_cgi_upload_path);
}
int											Config::get_cgi_timeout() const
{
	return (_cgi_timeout);
}


int		Config::set_root(const std::string& value)
{
	if (value.size() <= 0)
		return (1);
	_root = value;
	return (0);
}

#include "Color.hpp"

int		Config::set_client_max_body_size(const std::string& value)
{
	if (value.size() <= 0)
		return (1);
	std::stringstream ss(value);
	size_t nb;
	int mult = 1;
	if (ss >> nb)
	{
		char unit;
		if (ss >> unit)
		{
			char extra;
			if (ss >> extra)
				return (1);
			if (std::toupper(unit) == 'K')
				mult = 1024;
			if (std::toupper(unit) == 'M')
				mult = 1024 * 1024;
			if (std::toupper(unit) == 'G')
				mult = 1024 * 1024 * 1024;
		}
		_client_max_body_size = mult * nb;
	}
	else
		return (1);
	return (0);
}

int		Config::set_error_pages(const std::string& value)
{
	std::vector<int> key;
	std::string path;
	std::stringstream ss(value);
	int tmp = -1;
	while (ss >> tmp)
	{
		key.push_back(tmp);
	}
	if (tmp == -1)
		return (1);
	ss.clear();
	if (ss >> path)
	{
		char extra;
		if (ss >> extra)
			return  (1);
		for (std::vector<int>::iterator it = key.begin(); it != key.end(); it++)
		{
			_error_pages[*it] = path;
		}
	}
	return (0);
}