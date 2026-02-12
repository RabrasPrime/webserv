#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <vector>
#include <map>
#include <iostream>

class Config
{
	public:
		Config(){}
		~Config(){}

	private:
		std::string							_root;
		size_t								_client_max_body_size;
		std::map<int, std::string>			_error_pages;
		std::vector<std::string>			_methods;
		bool								_auto_index;
		std::vector<std::string>			_indexes;
		std::string							_upload_store;
		bool								_cgi_enabled;
		std::map<std::string, std::string>	_cgi_ext;
		std::string							_cgi_working_dir;
		std::string							_cgi_upload_path;
		int									_cgi_timeout;

	public:
		// GET
		const std::string							get_root() const;
		size_t										get_client_max_body_size() const;
		const std::map<int, std::string>			get_error_pages() const;
		const std::vector<std::string>				get_methods() const;
		bool										get_auto_index() const;
		const std::vector<std::string>				get_indexes() const;
		const std::string							get_upload_store() const;
		bool										get_cgi_enabled() const;
		const std::map<std::string, std::string>	get_cgi_ext() const;
		const std::string							get_cgi_working_dir() const;
		const std::string							get_cgi_upload_path() const;
		int											get_cgi_timeout() const;

		// SET
};

#endif