#pragma once

#include <string>

class User {
	private:
		const int			fd ;
		std::string			nickname ;
		const std::string	username ;

	public:
		User(int fd, const std::string &nickname, const std::string &username) ;


		const int			 getFd()		const	{ return this->fd ; } ;
		const std::string	&getNickname()	const	{ return this->nickname ; } ;
		const std::string	&getUsername()	const	{ return this->username ; } ;

		void setNickname(const std::string &newNickname) { this->nickname = newNickname ; } ;
} ;
