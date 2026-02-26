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
	_alias = loc._alias;
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
	_return_code = loc._return_code;
	_return_path = loc._return_path;

	_use_alias = loc._use_alias;

	_is_set_root = loc._is_set_root;
	_is_set_alias = loc._is_set_alias;
	_is_set_client_max_body_size = loc._is_set_client_max_body_size;
	_is_set_error_pages = loc._is_set_error_pages;
	_is_set_methods = loc._is_set_methods;
	_is_set_auto_index = loc._is_set_auto_index;
	_is_set_indexes = loc._is_set_indexes;
	_is_set_upload_path = loc._is_set_upload_path;
	_is_set_cgi_enabled = loc._is_set_cgi_enabled;
	_is_set_cgi_ext = loc._is_set_cgi_ext;
	_is_set_cgi_working_dir = loc._is_set_cgi_working_dir;
	_is_set_cgi_upload_path = loc._is_set_cgi_upload_path;
	_is_set_cgi_timeout = loc._is_set_cgi_timeout;
	_is_set_return = loc._is_set_return;
	_is_set_use_alias = loc._is_set_use_alias;
	return (*this);
}


int Location::fill_location_config(std::ifstream& file, std::string& line, int indent)
{
	while (std::getline(file, line))
	{
		if (count_char(line, '\t') < indent + 1)
		{
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

void Location::heritage_from_server(const Server& serv)
{
	if (!_is_set_root)
		_root = serv.get_root();
	if (!_is_set_alias)
		_alias = serv.get_alias();
	if (!_is_set_client_max_body_size)
		_client_max_body_size = serv.get_client_max_body_size();
	if (!_is_set_error_pages)
		_error_pages = serv.get_error_pages();
	if (!_is_set_methods)
		_methods = serv.get_methods();
	if (!_is_set_auto_index)
		_auto_index = serv.get_auto_index();
	if (!_is_set_indexes)
		_indexes = serv.get_indexes();
	if (!_is_set_upload_path)
		_upload_path = serv.get_upload_path();
	if (!_is_set_cgi_enabled)
		_cgi_enabled = serv.get_cgi_enabled();
	if (!_is_set_cgi_ext)
		_cgi_ext = serv.get_cgi_ext();
	if (!_is_set_cgi_working_dir)
		_cgi_working_dir = serv.get_cgi_working_dir();
	if (!_is_set_cgi_upload_path)
		_cgi_upload_path = serv.get_cgi_upload_path();
	if (!_is_set_cgi_timeout)
		_cgi_timeout = serv.get_cgi_timeout();
	if (!_is_set_return)
	{
		_return_code = serv.get_return_code();
		_return_path = serv.get_return_path();
	}
	if (!_is_set_use_alias)
		_use_alias = serv.get_use_alias();
}