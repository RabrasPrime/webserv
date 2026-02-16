//
// Created by tjooris on 2/16/26.
//

#include "../../includes/Client.hpp"
#include "../../includes/Color.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

// Constructor
Client::Client(int fd, Server *server)
	: _fd(fd),
	  _server(server),
	  _state(READING),
	  _last_activity(time(NULL)),
	  _keep_alive(true),
	  _timeout(30),  // 30 seconds default timeout
	  _is_connected(true),
	  _bytes_received(0),
	  _bytes_sent(0),
	  _request_count(0),
	  _response_status_code(200)
{
}

// Destructor
Client::~Client()
{
	close_connection();
}

// ============================================================================
// GETTERS - Basic Info
// ============================================================================

int Client::get_fd() const
{
	return (_fd);
}

Server* Client::get_server() const
{
	return (_server);
}

// ============================================================================
// GETTERS - Request Data
// ============================================================================

const std::string& Client::get_request_buffer() const
{
	return (_request_buffer);
}

const std::string& Client::get_method() const
{
	return (_http_method);
}

const std::string& Client::get_path() const
{
	return (_request_path);
}

const std::string& Client::get_http_version() const
{
	return (_http_version);
}

const std::string& Client::get_request_body() const
{
	return (_request_body);
}

std::string Client::get_request_header(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _request_headers.find(key);
	if (it != _request_headers.end())
		return (it->second);
	return ("");
}

// ============================================================================
// GETTERS - State
// ============================================================================

ClientState Client::get_state() const
{
	return (_state);
}

bool Client::is_connected() const
{
	return (_is_connected);
}

bool Client::is_keep_alive() const
{
	return (_keep_alive);
}

bool Client::is_timeout() const
{
	time_t current_time = time(NULL);
	return ((current_time - _last_activity) > _timeout);
}

// ============================================================================
// SETTERS - Request Handling
// ============================================================================

void Client::append_to_request_buffer(const std::string& data)
{
	_request_buffer += data;
	_bytes_received += data.length();
	update_activity();
}

void Client::clear_request_buffers()
{
	_request_buffer.clear();
	_http_method.clear();
	_request_path.clear();
	_http_version.clear();
	_request_headers.clear();
	_request_body.clear();
	_request_count++;
}

int Client::parse_request()
{
	// Find the end of headers (double CRLF)
	size_t header_end = _request_buffer.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return (-1);  // Headers not complete

	std::string headers_section = _request_buffer.substr(0, header_end);
	std::istringstream header_stream(headers_section);
	std::string line;

	// Parse request line (GET /path HTTP/1.1)
	if (std::getline(header_stream, line))
	{
		std::istringstream request_line(line);
		request_line >> _http_method >> _request_path >> _http_version;
	}

	// Parse headers
	while (std::getline(header_stream, line))
	{
		// Remove carriage return if present
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		size_t colon_pos = line.find(":");
		if (colon_pos != std::string::npos)
		{
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);

			// Trim leading/trailing whitespace from value
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);

			_request_headers[key] = value;
		}
	}

	// Extract body if present
	size_t body_start = header_end + 4;  // Skip the \r\n\r\n
	if (body_start < _request_buffer.length())
	{
		_request_body = _request_buffer.substr(body_start);
	}

	// Check for keep-alive
	std::string connection = get_request_header("Connection");
	if (connection == "close")
		_keep_alive = false;
	else if (_http_version == "HTTP/1.0")
		_keep_alive = false;

	return (0);
}

// ============================================================================
// SETTERS - Response Handling
// ============================================================================

void Client::set_response_status(int status_code)
{
	_response_status_code = status_code;
}

void Client::set_response_body(const std::string& body)
{
	_response_buffer = body;
}

void Client::add_response_header(const std::string& key, const std::string& value)
{
	_response_headers[key] = value;
}

void Client::build_http_response()
{
	std::stringstream response;

	// Status line
	std::string status_message;
	if (_response_status_code == 200) status_message = "OK";
	else if (_response_status_code == 201) status_message = "Created";
	else if (_response_status_code == 204) status_message = "No Content";
	else if (_response_status_code == 301) status_message = "Moved Permanently";
	else if (_response_status_code == 302) status_message = "Found";
	else if (_response_status_code == 304) status_message = "Not Modified";
	else if (_response_status_code == 400) status_message = "Bad Request";
	else if (_response_status_code == 401) status_message = "Unauthorized";
	else if (_response_status_code == 403) status_message = "Forbidden";
	else if (_response_status_code == 404) status_message = "Not Found";
	else if (_response_status_code == 405) status_message = "Method Not Allowed";
	else if (_response_status_code == 413) status_message = "Payload Too Large";
	else if (_response_status_code == 500) status_message = "Internal Server Error";
	else if (_response_status_code == 501) status_message = "Not Implemented";
	else if (_response_status_code == 503) status_message = "Service Unavailable";
	else status_message = "Unknown";

	response << "HTTP/1.1 " << _response_status_code << " " << status_message << "\r\n";

	// Add default headers
	if (_response_headers.find("Content-Type") == _response_headers.end())
		_response_headers["Content-Type"] = "text/html";
	if (_response_headers.find("Content-Length") == _response_headers.end())
		_response_headers["Content-Length"] = std::to_string(_response_buffer.length());
	if (_response_headers.find("Connection") == _response_headers.end())
		_response_headers["Connection"] = _keep_alive ? "keep-alive" : "close";

	// Add all headers
	for (std::map<std::string, std::string>::iterator it = _response_headers.begin();
		 it != _response_headers.end(); ++it)
	{
		response << it->first << ": " << it->second << "\r\n";
	}

	response << "\r\n";
	response << _response_buffer;

	_response_buffer = response.str();
	_bytes_sent += _response_buffer.length();
}

const std::string& Client::get_response_buffer() const
{
	return (_response_buffer);
}

void Client::clear_response_buffer()
{
	_response_buffer.clear();
	_response_headers.clear();
	_response_status_code = 200;
}

bool Client::is_response_ready() const
{
	return (!_response_buffer.empty());
}

// ============================================================================
// SETTERS - State Management
// ============================================================================

void Client::set_state(ClientState state)
{
	_state = state;
}

void Client::update_activity()
{
	_last_activity = time(NULL);
}

void Client::close_connection()
{
	_is_connected = false;
	if (_fd >= 0)
	{
		close(_fd);
		_fd = -1;
	}
}

void Client::enable_keep_alive(bool enable)
{
	_keep_alive = enable;
}

// ============================================================================
// Debugging
// ============================================================================

void Client::print_request_info() const
{
	std::cout << _BOLD << _CYAN << "\n=== CLIENT REQUEST INFO ===" << _END << std::endl;
	std::cout << "FD: " << _fd << std::endl;
	std::cout << "Method: " << _http_method << std::endl;
	std::cout << "Path: " << _request_path << std::endl;
	std::cout << "HTTP Version: " << _http_version << std::endl;
	std::cout << "Body Length: " << _request_body.length() << " bytes" << std::endl;
	std::cout << "Keep-Alive: " << (_keep_alive ? "Yes" : "No") << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = _request_headers.begin();
		 it != _request_headers.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
	std::cout << _BOLD << _CYAN << "==========================\n" << _END << std::endl;
}

