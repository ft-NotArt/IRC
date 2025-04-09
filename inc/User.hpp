#pragma once

#include <string>

class User {
	private:
		const int	fd ;
		bool		inCAP ;
		bool		registered ;
		std::string	password ;
		std::string	nickname ;
		std::string	username ;
		std::string	hostname ;

	public:
		User(int fd) : fd(fd), inCAP(false), registered(false) {} ;

		int			 		getFd()			const	{ return this->fd ; } ;
		bool				isInCAP()		const	{ return this->inCAP ; } ;
		bool				isRegistered()	const	{ return this->registered ; } ;
		const std::string	&getPassword()	const	{ return this->password ; } ;
		const std::string	&getNickname()	const	{ return this->nickname ; } ;
		const std::string	&getUsername()	const	{ return this->username ; } ;
		const std::string	&getHostname()	const	{ return this->hostname ; } ;
		const std::string	getFullname()	const	{ return this->getNickname() + "!" + this->getUsername() + "@" + this->getHostname() ; } ;

		void				setInCAP(bool inCAP)						{ this->inCAP = inCAP ; } ;
		void				setRegistered(bool registered)				{ this->registered = registered ; } ;
		void				setPassword(const std::string &newPassword)	{ this->password = newPassword ; } ;
		void				setNickname(const std::string &newNickname)	{ this->nickname = newNickname ; } ;
		void				setUsername(const std::string &newUsername)	{ this->username = newUsername ; } ;
		void				setHostname(const std::string &newHostname)	{ this->hostname = newHostname ; } ;
} ;
