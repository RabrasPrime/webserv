#include <Utils.hpp>
#include "Color.hpp"

int count_char(const std::string& line, char c)
{
	int i = 0;
	while (line[i] == c)
		i++;
	return (i);
}

std::string print_warning(const std::string& title, const std::string& content, const std::string& value)
{
	std::cout << "⚠️  "  _ORANGE _BOLD _UNDER << title << _END << " : " << _ORANGE << content << _END << "  ➡️   \"" << value << "\"" << std::endl;
	return "";
}