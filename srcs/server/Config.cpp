#include "Utils.hpp"
#include "Config.hpp"
#include <sstream>

Config::Config()
:_client_max_body_size(CLIENT_MAX_BODY_SIZE_DEFAULT)
,_methods(0)
,_auto_index(0)
,_cgi_enabled(0)
,_cgi_timeout(10)
,_is_set_root(false)
,_is_set_client_max_body_size(false)
,_is_set_error_pages(false)
,_is_set_methods(false)
,_is_set_auto_index(false)
,_is_set_indexes(false)
,_is_set_upload_path(false)
,_is_set_cgi_enabled(false)
,_is_set_cgi_ext(false)
,_is_set_cgi_working_dir(false)
,_is_set_cgi_upload_path(false)
,_is_set_cgi_timeout(false)
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
const std::string							Config::get_upload_path() const
{
	return (_upload_path);
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
	else
	{
		for (std::vector<int>::iterator it = key.begin(); it != key.end(); it++)
		{
			_error_pages.erase(*it);
		}
	}
	return (0);
}
int		Config::set_methods(const std::string& value)
{
	std::stringstream ss(value);

	std::string word;
	while (ss >> word)
	{
		if (word == "GET")
			_methods |= METHOD_GET;
		else if (word == "POST")
			_methods |= METHOD_POST;
		else if (word == "DELETE")
			_methods |= METHOD_DELETE;
		else
			return (1);
	}
	return (0);
}
int		Config::set_auto_index(const std::string& value)
{
	std::stringstream ss(value);

	std::string word;
	if (ss >> word)
	{
		char extra;
		if (ss >> extra)
			return (1);
		if (word == "1" || word == "true" || word == "True")
			_auto_index = 1;
		else if (word == "0" || word == "false" || word == "False")
			_auto_index = 0;
		else
			return (1);
	}
	else
		return (1);
	return (0);
}
int		Config::set_indexes(const std::string& value)
{
	std::stringstream ss(value);

	std::string word;
	while (ss >> word)
	{
		_indexes.push_back(word);
	}
	return (0);
}
int		Config::set_upload_path(const std::string& value)
{
	std::stringstream ss(value);
	std::string path;

	if (ss >> path)
	{
		char extra;
		if (ss >> extra)
			return (1);
		_upload_path = path;
	}
	return (0);
}
int		Config::set_cgi_enabled(const std::string& value)
{
	std::stringstream ss(value);

	std::string word;
	if (ss >> word)
	{
		char extra;
		if (ss >> extra)
			return (1);
		if (word == "1" || word == "true" || word == "True")
			_cgi_enabled = 1;
		else if (word == "0" || word == "false" || word == "False")
			_cgi_enabled = 0;
		else
			return (1);
	}
	else
		return (1);
	return (0);
}



void Config::fill_config(const std::string& key, const std::string& value)
{
	if (key == "root")
		set_root(value) ? print_warning("Warning","Invalid value on root !"): (_is_set_root = true,"");
	if (key == "client_max_body_size")
		set_client_max_body_size(value) ? print_warning("Warning","Invalid value on client_max_body_size !"): (_is_set_client_max_body_size = true,"");
	if (key == "error_pages")
		set_error_pages(value) ? print_warning("Warning","Invalid value on error_pages !"): (_is_set_error_pages = true,"");
	if (key == "methods")
		set_methods(value) ? print_warning("Warning","Invalid value on methods !"): (_is_set_methods = true,"");
	if (key == "auto_index")
		set_auto_index(value) ? print_warning("Warning","Invalid value on auto_index !"): (_is_set_auto_index = true,"");
	if (key == "index")
		set_indexes(value) ? print_warning("Warning","Invalid value on index !"): (_is_set_indexes = true,"");
	if (key == "upload_path")
		set_upload_path(value) ? print_warning("Warning","Invalid value on upload_path !"): (_is_set_upload_path = true,"");
	if (key == "cgi_enabled")
		set_cgi_enabled(value) ? print_warning("Warning","Invalid value on cgi_enabled !"): (_is_set_upload_path = true,"");
}

std::ostream& operator<<(std::ostream& out, const Config&  config)
{
	out << "Client Max Body Size : " << config._client_max_body_size << std::endl;
	out << "Error Pages : " << std::endl;
	for (std::map<int, std::string>::const_iterator it = config._error_pages.begin(); it != config._error_pages.end();it++)
		out << "\t" << it->first << " " << it->second << std::endl;
	out << "Methods : "	<< (METHOD_DELETE & config._methods ? "DELETE " : "")
						<< (METHOD_GET & config._methods ? "GET " : "")
						<< (METHOD_POST & config._methods ? "POST " : "") << std::endl;
	out << "Auto Index : " << config._auto_index << std::endl;
	out << "Index : ";
	for (std::vector<std::string>::const_iterator it = config._indexes.begin(); it != config._indexes.end();it++)
		out << *it << " ";
	out << std::endl;
	out << "Upload Path : " << config._upload_path << std::endl;
	return (out);
}