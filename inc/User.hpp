#pragma once

#include <string>

class User {
	private:
		const int	fd ;
		bool		authenticated ;
		std::string	nickname ;
		std::string	username ;

	public:
		User(int fd) ;


		int			 		getFd()				const	{ return this->fd ; } ;
		bool				isAuthenticated()	const	{ return this->authenticated ; } ;
		const std::string	&getNickname()		const	{ return this->nickname ; } ;
		const std::string	&getUsername()		const	{ return this->username ; } ;

		void				setAuthenticated(bool auth) { this->authenticated = auth ; } ;
		void				setNickname(const std::string &newNickname) { this->nickname = newNickname ; } ;
		void				setUsername(const std::string &newUsername) { this->username = newUsername ; } ;
} ;
