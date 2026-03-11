#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "Location.hpp"
#include <fstream>

struct HttpRequest
{
	size_t		total_size;
	size_t		chunked_size;
	int			chunked;
	Location*	loc;
	bool		location_match;
	size_t		ContentLength;
	int			end_head;

	int 		method;
	std::string raw_path;
	std::string path;
	std::string version;
	std::string type;
	std::vector<std::string> env;
	std::map<std::string, std::string> headers;
	std::map<std::string, std::vector<std::string> > mult;
	// std::string body;
	std::vector<unsigned char> body;
	Server *tartgetServ; 
	// dans Location
	std::map<int, std::string>			error_pages;
	int									methods;
	int									ErrorCode;
	bool								auto_index;
	std::vector<std::string>			indexes;
	size_t								maxSize;
	std::string 						root;
	std::map<std::string, std::string>	cgi_ext;

	std::string 						queryString;
	std::ofstream*	outFile;
	// int pipefdIn;
	// int pipefdOut;
	int pipeOut[2];
	int pipeIn[2];
	pid_t cgi_pid;
	bool isCgi;
	bool foundHeader;
	std::string str;
	std::vector<unsigned char> dataCgi;
	int fd;

	size_t total_send;

	// HttpRequest()
	// : pipefdIn(-1)
	// , pipefdOut(-1)
	// {}
};

#endif
