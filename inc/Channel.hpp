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

typedef enum c_perms {
	OPERATOR	= 0b10000000,
	INVITED		= 0b00010000,
	BANNED		= 0b00000001,
	NORMAL		= 0b00000000
} t_perms ;

class Channel {
	private:
		const Server					&server ;
		const std::string				name ;
		std::string						password ;
		std::string						topic ;
		std::pair<const User *, time_t>	topic_change ;
		std::set<const User *>			users ;
		std::map<const User *, uint8_t>	perms ;
		std::string						modesString ;
		std::string						modesArgs ;
		int								max_users ;
		bool							invite_only ;
		bool							topic_restrict ;

	public:
		/***
		 * @warning Throw exception on invalid name
		 */
		Channel(const std::string &name, const Server &server, const User *creator) ;

		const std::string						&getName()			const	{ return this->name ; } ;
		const std::string						&getPassword()		const	{ return this->password ; } ;
		const std::string						&getTopic()			const	{ return this->topic ; } ;
		const std::pair<const User *, time_t>	&getTopicChange()	const	{ return this->topic_change ; } ;
		int										getMaxUsers()		const	{ return this->max_users ; } ;
		bool									getTopicRestrict()	const 	{ return this->topic_restrict ; } ;
		std::string								getUsers()			const	;
		int										getUsersNb()		const	{ return this->users.size() ; } ;
		std::string								getModesString()	const	{ return this->modesString ; } ;
		std::string								getModesArgs()		const	{ return this->modesArgs ; } ;

		bool									isUserIn(const User *user)	const ;

		void	setPassword(const std::string &password)					{ this->password = password ; } ;
		void	setMaxUsers(int max_users)									{ this->max_users = max_users ; } ;
		void	setInviteOnly(bool invite_only)								{ this->invite_only = invite_only ; } ;
		void	setTopicRestrict(bool topic_restrict)						{ this->topic_restrict = topic_restrict ; } ;
		void	addModesString(const std::string &modesString)				{ this->modesString += modesString ; } ;
		void	addModesArgs(const std::string &modesArgs)					{ this->modesArgs += modesArgs ; } ;

		bool	hasPerms(const User *user, uint8_t perms) ;
		void	addPerms(const User *user, uint8_t perms) ;
		void	removePerms(const User *user, uint8_t perms) ;

		void	join(const User *user, const std::string &password) ;
		void	leave(const User *user, const std::string &msg) ;
		void	kick(const User *user, const User *kicked, const std::string &msg) ;
		void	ban(const User *user, const User *banned) ;
		void	invite(const User *user, const User *invited) ;

		void	changeTopic(const User *user, const std::string &topic) ;
		void	sendTopic(const User *user) ;

		void	sendMsg(const User *user, const std::string &text) const ;
} ;
