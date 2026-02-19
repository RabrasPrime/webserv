#include "Utils.hpp"
#include "Config.hpp"
#include "Color.hpp"
#include <sstream>

Config::Config()
:_client_max_body_size(CLIENT_MAX_BODY_SIZE_DEFAULT)
,_methods(0)
,_auto_index(0)
,_cgi_enabled(0)
,_cgi_timeout(10)
,_return_code(0)
,_is_set_root(false)
,_is_set_alias(false)
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
,_is_set_return(false)
{
}
Config::~Config()
{

}


const std::string							Config::get_root() const
{
	return (_root);
}
const std::string							Config::get_alias() const
{
	return (_alias);
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
int											Config::get_return_code() const
{
	return (_return_code);
}
std::string									Config::get_return_path() const
{
	return (_return_path);
}


int		Config::set_root(const std::string& value)
{
	std::stringstream ss(value);
	std::string path;
	if (!(ss >> path))
		return (1);
	char extra;
	if (ss >> extra)
		return (1);
	_root = path;
	return (0);
}
int		Config::set_alias(const std::string& value)
{
	std::stringstream ss(value);
	std::string path;
	if (!(ss >> path))
		return (1);
	char extra;
	if (ss >> extra)
		return (1);
	_alias = path;
	return (0);
}
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
int		Config::set_cgi_ext(const std::string& value)
{
	std::stringstream ss(value);
	std::vector<std::string> list;
	std::string str;
	while (ss >> str)
	{
		list.push_back(str);
	}
	if (list.size() < 2)
		return (1);
	std::string path = list.back();
	list.pop_back();
	for (std::vector<std::string>::iterator it = list.begin();it != list.end();it++)
	{
		if ((*it)[0] == '.')
		{
			(*it) = &(*it)[1];
		}
		if (path != "off")
			_cgi_ext[*it] = path;
		else
			_cgi_ext.erase(*it);
	}
	return (0);
}
int		Config::set_cgi_working_dir(const std::string& value)
{
	std::stringstream ss(value);
	std::string path;

	if (ss >> path)
	{
		char extra;
		if (ss >> extra)
			return (1);
		_cgi_working_dir = path;
	}
	else
		return (1);
	return (0);
}
int		Config::set_cgi_upload_path(const std::string& value)
{
	std::stringstream ss(value);
	std::string path;

	if (ss >> path)
	{
		char extra;
		if (ss >> extra)
			return (1);
		_cgi_upload_path = path;
	}
	else
		return (1);
	return (0);
}
int		Config::set_cgi_timeout(const std::string& value)
{
	std::stringstream ss(value);
	int time;

	if (ss >> time)
	{
		char extra;
		if (ss >> extra)
			return (1);
		_cgi_timeout = time;
	}
	else
		return (1);
	return (0);
}
int		Config::set_return(const std::string& value)
{
	std::stringstream ss(value);
	int code;
	std::string path;
	if (!(ss >> code))
		return (1);
	if (ss >> path)
	{
		char extra;
		if (ss >> extra)
			return (1);
		_return_code = code;
		_return_path = path;
	}
	else
		return (1);
	return (0);
}

void Config::fill_config(const std::string& key, const std::string& value)
{
	if (key == "root")
		set_root(value) ? print_warning("Warning","Invalid value on root !", value): (_is_set_root = true,"");
	if (key == "alias")
		set_alias(value) ? print_warning("Warning","Invalid value on alias !", value): (_is_set_alias = true,"");
	if (key == "client_max_body_size")
		set_client_max_body_size(value) ? print_warning("Warning","Invalid value on client_max_body_size !", value): (_is_set_client_max_body_size = true,"");
	if (key == "error_pages")
		set_error_pages(value) ? print_warning("Warning","Invalid value on error_pages !", value): (_is_set_error_pages = true,"");
	if (key == "methods")
		set_methods(value) ? print_warning("Warning","Invalid value on methods !", value): (_is_set_methods = true,"");
	if (key == "auto_index")
		set_auto_index(value) ? print_warning("Warning","Invalid value on auto_index !", value): (_is_set_auto_index = true,"");
	if (key == "index")
		set_indexes(value) ? print_warning("Warning","Invalid value on index !", value): (_is_set_indexes = true,"");
	if (key == "upload_path")
		set_upload_path(value) ? print_warning("Warning","Invalid value on upload_path !", value): (_is_set_upload_path = true,"");
	if (key == "cgi_enabled")
		set_cgi_enabled(value) ? print_warning("Warning","Invalid value on cgi_enabled !", value): (_is_set_cgi_enabled = true,"");
	if (key == "cgi_ext")
		set_cgi_ext(value) ? print_warning("Warning","Invalid value on cgi_ext !", value): (_is_set_cgi_ext = true,"");
	if (key == "cgi_working_dir")
		set_cgi_working_dir(value) ? print_warning("Warning","Invalid value on cgi_working_dir !", value): (_is_set_cgi_working_dir = true,"");
	if (key == "cgi_upload_path")
		set_cgi_upload_path(value) ? print_warning("Warning","Invalid value on cgi_upload_path !", value): (_is_set_cgi_upload_path = true,"");
	if (key == "cgi_timeout")
		set_cgi_timeout(value) ? print_warning("Warning","Invalid value on cgi_timeout !", value): (_is_set_cgi_timeout = true,"");
	if (key == "return")
		set_return(value) ? print_warning("Warning","Invalid value on return !", value): (_is_set_return = true,"");
}

std::ostream& operator<<(std::ostream& out, const Config&  config)
{
	out << BLUE << "Root : " << PURPLE << config._root << RESET << std::endl;
	out << BLUE << "Alias : " << PURPLE << config._alias << RESET << std::endl;
	out << BLUE << "Client Max Body Size : " << PURPLE << config._client_max_body_size << RESET << std::endl;
	out << BLUE << "Error Pages : " << PURPLE << std::endl;
	for (std::map<int, std::string>::const_iterator it = config._error_pages.begin(); it != config._error_pages.end();it++)
		out << BLUE << "\t" << it->first << PURPLE << " " << it->second << RESET << std::endl;
	out << BLUE << "Methods : " << PURPLE << (METHOD_DELETE & config._methods ? "DELETE " : "")
						<< (METHOD_GET & config._methods ? "GET " : "")
						<< (METHOD_POST & config._methods ? "POST " : "") << RESET << std::endl;
	out << BLUE << "Auto Index : " << PURPLE << config._auto_index << RESET << std::endl;
	out << BLUE << "Index : " << PURPLE;
	for (std::vector<std::string>::const_iterator it = config._indexes.begin(); it != config._indexes.end();it++)
		out << *it << " ";
	out << RESET << std::endl;
	out << BLUE << "Upload Path : " << PURPLE << config._upload_path << RESET << std::endl;
	out << BLUE << "Cgi Enabled : " << PURPLE << config._cgi_enabled << RESET << std::endl;
	out << BLUE << "Cgi Ext : " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = config._cgi_ext.begin();it != config._cgi_ext.end();it++)
		out << BLUE << "\t" << it->first << PURPLE << " " << it->second << RESET << std::endl;
	out << BLUE << "Cgi Working Dir : " << PURPLE << config._cgi_working_dir << RESET << std::endl;
	out << BLUE << "Cgi Upload Path : " << PURPLE << config._cgi_upload_path << RESET << std::endl;
	out << BLUE << "Cgi Timeout : " << PURPLE << config._cgi_timeout << RESET << std::endl;
	out << BLUE << "Return : " << PURPLE << config._return_code << " " << config._return_path << RESET << std::endl;
	return (out);
}