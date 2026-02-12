#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <exception>
#include <map>
#include <dirent.h>
#include "httpRequest.hpp"

class httpResponse
{
	public:
		httpResponse();
		// httpResponse(const HttpRequest &req);
		~httpResponse(){}
	
	private:
		int _statusCode;
		std::string _version;
		std::string _statusMsg;
		std::map<std::string, std::string> _headers;
		std::string _body;
		std::map<int, std::string> ErrorMsg;
		// std::string _method;

	public:
		void handleRequest(HttpRequest &req);
		std::string convertFinalResponse();
		void exeGet(HttpRequest &req);
		void fillHeaders(std::map<std::string, std::string> &headers);
		void fillBody(std::string &path);
		void exePost();
		void exeDelete();

		std::string getVersion() const;
		std::string getStatusMsg() const;
		std::string getBody() const;
		std::string getMethod() const;
		int getStatusCode() const;
		std::map<std::string, std::string> getHeaders() const;
};
#endif

// void  BitcoinExchange::openFile(const char *filename, std::ifstream& file){
	// struct stat	s;
	// int checkErr = 0;
	// file.open(filename);
	// checkErr = lstat(filename, &s);
	// if (!file.is_open() || checkErr != 0 || (checkErr == 0 && S_ISDIR(s.st_mode)))
	// {
		// std::cout << RED "Error: " << filename << ", " RESET;
		// throw ErrorOpenFileException();
	// }
// }

/*_statusCode = 404;
		_body = "File not Found";
		return ;*/


// int	get_nb_file(char *wildcards)
// {
// 	DIR				*current_dir;
// 	struct dirent	*read_file;
// 	int				nb_file;

// 	nb_file = 0;
// 	errno = 0;
// 	current_dir = NULL;
// 	if (open_dir(&current_dir) == false)
// 		return (-1);
// 	read_file = readdir(current_dir);
// 	while (1)
// 	{
// 		read_file = readdir(current_dir);
// 		if (errno != 0)
// 			return (-3);
// 		else if (read_file == NULL)
// 			break ;
// 		if ((wildcards[0] == DOT || read_file->d_name[0] != DOT)
// 			&& check_match(read_file->d_name, wildcards) == true)
// 			++nb_file;
// 	}
// 	if (close_dir(&current_dir) == false)
// 		return (-2);
// 	return (nb_file);
// }



