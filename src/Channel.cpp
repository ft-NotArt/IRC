/* Includes */

#include "Channel.hpp"


/* Constructor */

Channel::Channel(const std::string &name, const Server &server) : server(server), name(name) {
	this->password = "" ;
	this->topic = "" ;
	this->topic_change.first = NULL ;
	this->topic_change.second = std::time(NULL) ;
	this->max_users = -1 ;
	this->invite_only = false ;
}


/* Get */

std::string Channel::getUsers() const {
	std::string res ;
	for (std::set<const User *>::iterator it = this->users.begin(); it != this->users.end() ; it++) {
		try {
			if (this->perms.at(*it) & OPERATOR)
				res += '@' ;
			res += (*it)->getNickname() + ' ' ;
		} catch(const std::exception& e) {
			continue ;
		}
	}

	return res ;
}


/* Methods */

void Channel::join(const User *user, const std::string &password) {

	// TODO: Possibly checking for banned

	if (this->invite_only) {
		try {
			if (!(this->perms.at(user) & INVITED))
				; // return
		} catch(const std::exception& e) {
			; // throw not invited err
		}
	}

	if (this->max_users != -1 && (int) this->users.size() == this->max_users) {
		; // throw too many users err
	}

	if (this->password != "" && password != this->password) {
		; // throw wrong pswd err
	}

	// TODO PUT BACK THIS CODE WITH FIX (Blocked Compilation)
	this->users.insert(user) ;
	try {
		this->perms.at(user) &= !INVITED ;
	} catch(const std::exception& e) {
		this->perms[user] = NORMAL ;
	}


	// TODO: send JOIN msg

	if (this->topic != "") {
		this->server.RPL_TOPIC(user, *this) ;
		this->server.RPL_TOPICWHOTIME(user, *this) ;
	} else
		this->server.RPL_NOTOPIC(user, *this) ;

	this->server.RPL_NAMREPLY(user, *this) ;
	this->server.RPL_ENDOFNAMES(user, *this) ;
}
