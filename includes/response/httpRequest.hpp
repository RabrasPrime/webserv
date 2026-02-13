#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

struct HttpRequest
{
	std::string method;
	std::string path;
	std::string version;
	std::string type;
	std::map<std::string, std::string> headers;
	std::string body;

	// dans Location
	std::map<int, std::string>	error_pages; 
	std::vector<std::string>	methods;
	bool						auto_index;
	std::vector<std::string>	indexes; 
	bool autoIndex;
};

#endif



