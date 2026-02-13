#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

struct HttpRequest
{
	std::string method;
	std::string path; // get le body 
	std::string version;
	std::string type;
	std::map<std::string, std::string> headers;
	std::string body;
	bool autoIndex;
};

#endif



