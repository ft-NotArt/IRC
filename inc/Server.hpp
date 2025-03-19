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

		const std::string			&getPassword()	const	{ return this->password ; } ;
		int			 				getPort()			const	{ return this->port ; } ;
		std::map<int, const User *>	getUsers()	const	{ return this->users ; } ;

		void	addUser(int fd, const User *user) ;
};
