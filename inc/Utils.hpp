#pragma once

#include <string>

#define PATTERN " \f\n\r\t\v"

std::string trim_left(const std::string& str) {
	std::size_t index = str.find_first_not_of(PATTERN) ;

	if (index == std::string::npos)
		return "" ;

	return str.substr(index) ;
}

std::string trim_right(const std::string& str) {
	return str.substr(0,str.find_last_not_of(PATTERN) + 1);
}

std::string trim(const std::string& str) {
	return trim_left(trim_right(str));
}
