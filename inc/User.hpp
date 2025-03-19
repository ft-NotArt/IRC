#pragma once

#include <string>

class User {
	private:
		const int	fd ;
		std::string	nickname ;
		std::string	username ;

	public:
		User(int fd) ;


		int			 		getFd()			const	{ return this->fd ; } ;
		const std::string	&getNickname()	const	{ return this->nickname ; } ;
		const std::string	&getUsername()	const	{ return this->username ; } ;

		void				setNickname(const std::string &newNickname) { this->nickname = newNickname ; } ;
		void				setUsername(const std::string &newUsername) { this->username = newUsername ; } ;
} ;
