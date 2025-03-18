#pragma once

#include "User.hpp"

#include <string>
#include <set>
#include <map>

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
		int								maxUsers = -1 ;

	public:
		void	addUser(const User *user, uint8_t perms) ;
} ;
