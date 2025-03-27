#pragma once

#include <string>

#define PATTERN " \f\n\r\t\v"

inline std::string trim_left(const std::string& str) {
	std::size_t index = str.find_first_not_of(PATTERN) ;

	if (index == std::string::npos)
		return "" ;

	return str.substr(index) ;
}

inline std::string trim_right(const std::string& str) {
	return str.substr(0,str.find_last_not_of(PATTERN) + 1);
}

inline std::string trim(const std::string& str) {
	return trim_left(trim_right(str));
}

inline void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty()) return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Advance position
	}
}
