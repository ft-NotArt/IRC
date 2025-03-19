#pragma once

#include "Server.hpp"
#include "User.hpp"

class Server ;
class User ;

#include <string>
#include <set>
#include <map>
#include <utility>
#include <ctime>
#include <stdint.h>

enum {
	OPERATOR	= 0b10000000,
	INVITED		= 0b00010000,
	BANNED		= 0b00000001,
	NORMAL		= 0b00000000
} ;

class Channel {
	private:
		Server					& const	server ;
		const std::string				name ;
		std::string						password ;
		std::string						topic ;
		std::pair<const User *, time_t>	topic_change ;
		std::set<const User *>			users ;
		std::map<const User *, uint8_t>	perms ;
		int								max_users ;
		bool							invite_only ;
		// bool							topic_restrict ;

	public:
	// TODO: There should be parsing on channel name (ex. starting with #), idk at which point of the code though

	// , topic_restrict(false)
		Channel(const std::string &name, Server & const server) ;

		const std::string						&getName()			const	{ return this->name ; } ;
		const std::string						&getPassword()		const	{ return this->password ; } ;
		const std::string						&getTopic()			const	{ return this->topic ; } ;
		const std::pair<const User *, time_t>	&getTopicChange()	const	{ return this->topic_change ; } ;
		int										getMaxUsers()		const	{ return this->max_users ; } ;
		std::string								getUsers()			const	;

		void	setPassword(const std::string &password)	{ this->password = password ; } ;
		void	setTopic(const std::string &topic)			{ this->topic = topic ; } ;
		void	setMaxUsers(int max_users)					{ this->max_users = max_users ; } ;

		void	join(const User *user, const std::string &password) ;
} ;
