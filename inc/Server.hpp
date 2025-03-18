#pragma once

#include "User.hpp"

#include <string>
#include <map>

class Server {
	private:
		const std::string			password ;
		const int					port ;
		std::map<int, const User *>	users ;

	public:
		Server(const std::string &password, const int port) ;

		void	addUser(int fd, const User *user) ;
};
