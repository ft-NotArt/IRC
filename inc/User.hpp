#pragma once

#include <string>

class User {
	private:
		const int	fd ;
		bool		requestCap ;
		std::string	password ;
		std::string	nickname ;
		std::string	username ;

	public:
		User(int fd) ;


		int			 		getFd()				const	{ return this->fd ; } ;
		bool				hasRequestCap()		const	{ return this->requestCap ; } ;
		const std::string	&getPassword()		const	{ return this->password ; } ;
		const std::string	&getNickname()		const	{ return this->nickname ; } ;
		const std::string	&getUsername()		const	{ return this->username ; } ;

		void				setRequestCap(bool cap) { this->requestCap = cap ; } ;
		void				setPassword(const std::string &newPassword) { this->password = newPassword ; } ;
		void				setNickname(const std::string &newNickname) { this->nickname = newNickname ; } ;
		void				setUsername(const std::string &newUsername) { this->username = newUsername ; } ;
} ;
