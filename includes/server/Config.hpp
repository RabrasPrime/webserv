#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <vector>
#include <map>
#include <iostream>

# define METHOD_GET 0b1
# define METHOD_POST 0b10
# define METHOD_DELETE 0b100
# define CLIENT_MAX_BODY_SIZE_DEFAULT 10 * 1024 * 1024 // 10M

class Config
{
	public:
		Config();
		~Config();

		void fill_config(const std::string& key, const std::string& value);
		friend std::ostream& operator<<(std::ostream& out, const Config& config);

	protected:
		std::string							_root;
		std::string							_alias;
		size_t								_client_max_body_size;
		std::map<int, std::string>			_error_pages;
		int									_methods;
		bool								_auto_index;
		std::vector<std::string>			_indexes;
		std::string							_upload_path;
		bool								_cgi_enabled;
		std::map<std::string, std::string>	_cgi_ext;
		std::string							_cgi_working_dir;
		std::string							_cgi_upload_path;
		int									_cgi_timeout;
		int									_return_code;
		std::string							_return_path;
		bool								_use_alias;

		bool								_is_set_root;
		bool								_is_set_alias;
		bool								_is_set_client_max_body_size;
		bool								_is_set_error_pages;
		bool								_is_set_methods;
		bool								_is_set_auto_index;
		bool								_is_set_indexes;
		bool								_is_set_upload_path;
		bool								_is_set_cgi_enabled;
		bool								_is_set_cgi_ext;
		bool								_is_set_cgi_working_dir;
		bool								_is_set_cgi_upload_path;
		bool								_is_set_cgi_timeout;
		bool								_is_set_return;
		bool								_is_set_use_alias;

	public:
		// GET
		const std::string							get_root() const;
		const std::string							get_alias() const;
		size_t										get_client_max_body_size() const;
		const std::map<int, std::string>			get_error_pages() const;
		int											get_methods() const;
		bool										get_auto_index() const;
		const std::vector<std::string>				get_indexes() const;
		const std::string							get_upload_path() const;
		bool										get_cgi_enabled() const;
		const std::map<std::string, std::string>	get_cgi_ext() const;
		const std::string							get_cgi_working_dir() const;
		const std::string							get_cgi_upload_path() const;
		int											get_cgi_timeout() const;
		int											get_return_code() const;
		std::string									get_return_path() const;

		bool										get_use_alias() const;

		// SET
		int		set_root(const std::string& value);
		int		set_alias(const std::string& value);
		int		set_client_max_body_size(const std::string& value);
		int		set_error_pages(const std::string& value);
		int		set_methods(const std::string& value);
		int		set_auto_index(const std::string& value);
		int		set_indexes(const std::string& value);
		int		set_upload_path(const std::string& value);
		int		set_cgi_enabled(const std::string& value);
		int		set_cgi_ext(const std::string& value);
		int		set_cgi_working_dir(const std::string& value);
		int		set_cgi_upload_path(const std::string& value);
		int		set_cgi_timeout(const std::string& value);
		int		set_return(const std::string& value);
};

#endif