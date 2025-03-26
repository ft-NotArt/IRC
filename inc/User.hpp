#pragma once

#include <string>

class User {
	private:
		const int	fd ;
		bool		authenticated ;
		bool		requestCap ;
		std::string	nickname ;
		std::string	username ;

	public:
		User(int fd) ;


		int			 		getFd()				const	{ return this->fd ; } ;
		bool				isAuthenticated()	const	{ return this->authenticated ; } ;
		bool				hasRequestCap()		const	{ return this->requestCap ; } ;
		const std::string	&getNickname()		const	{ return this->nickname ; } ;
		const std::string	&getUsername()		const	{ return this->username ; } ;

		void				setAuthenticated(bool auth) { this->authenticated = auth ; } ;
		void				setRequestCap(bool cap) { this->requestCap = cap ; } ;
		void				setNickname(const std::string &newNickname) { this->nickname = newNickname ; } ;
		void				setUsername(const std::string &newUsername) { this->username = newUsername ; } ;
} ;
