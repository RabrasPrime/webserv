#include "Config.hpp"

const std::string							Config::get_root() const
{
	return (_root);
}
size_t								Config::get_client_max_body_size() const
{
	return (_client_max_body_size);
}
const std::map<int, std::string>			Config::get_error_pages() const
{
	return (_error_pages);
}
const std::vector<std::string>				Config::get_methods() const
{
	return (_methods);
}
bool									Config::get_auto_index() const
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
bool									Config::get_cgi_enabled() const
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
int									Config::get_cgi_timeout() const
{
	return (_cgi_timeout);
}
