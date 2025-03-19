#pragma once

#include "User.hpp"

#include <string>
#include <set>
#include <map>
#include <stdint.h>

enum {
	OPERATOR	= 0b10000000,
	INVITED		= 0b00010000,
	BANNED		= 0b00000001
} ;

class Channel
{
	private:
		std::string						password ;
		std::string						topic ;
		std::set<const User *>			users ;
		std::map<const User *, uint8_t>	perms ;
		int								maxUsers;

	public:
		Channel() : maxUsers(-1) {} ;

		const std::string				&getPassword()	const	{ return this->password ; } ;
		const std::string				&getTopic()		const	{ return this->topic ; } ;
		std::set<const User *>			getUsers()		const	{ return this->users ; } ;
		std::map<const User *, uint8_t>	getPerms()		const	{ return this->perms ; } ;
		int								getMaxUsers()	const	{ return this->maxUsers ; } ;

		void	setPassword(const std::string &password) { this->password = password ; } ;
		void	setTopic(const std::string &topic) { this->topic = topic ; } ;
		void	setMaxUsers(int maxUsers) { this->maxUsers = maxUsers ; } ;

		void	addUser(const User *user, uint8_t perms) ;
} ;
