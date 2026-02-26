#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

struct HttpRequest
{
	int 		method;
	std::string path;
	std::string version;
	std::string type;
	std::map<std::string, std::string> headers;
	std::map<std::string, std::vector<std::string> > mult;
	std::string body;

	// dans Location
	std::map<int, std::string>	error_pages;
	int							methods;
	int							ErrorCode;
	bool						auto_index;
	std::vector<std::string>	indexes;
	size_t						maxSize;
	std::string 				root;

	std::string 				queryString;
};

#endif

// root /usr/bin/
// alias /usr/bin/

// location /image/
// /usr/bin/image/script/hello.py?
// /usr/bin/hello.py
