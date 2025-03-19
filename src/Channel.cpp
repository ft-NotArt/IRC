/* Includes */

#include "Channel.hpp"


/* Constructor */

Channel::Channel(const std::string &name, Server & const server) : name(name), server(server) {
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

	// TODO: Which order to handle those checks ?

	if (this->password != "" && password != this->password) {
		; // throw wrong pswd err
	}

	if (this->max_users != -1 && (int) this->users.size() == this->max_users) {
		; // throw too many users err
	}

	if (this->invite_only) {
		try {
			if (!(this->perms.at(user) & INVITED)) // TODO: In current state of the code, this would probably end up throwing this error on client already in channel trying to join it | What about those mf ?
				; // throw not invited err
		} catch(const std::exception& e) {
			; // throw not invited err
		}
	}

	this->users.insert(user) ;
	this->perms[user] = NORMAL ;


	// TODO: send JOIN msg

	if (this->topic != "") {
		this->server.RPL_TOPIC(user, *this) ;
		this->server.RPL_TOPICWHOTIME(user, *this) ;
	} else
		this->server.RPL_NOTOPIC(user, *this) ;

	this->server.RPL_NAMREPLY(user, *this) ;
	this->server.RPL_ENDOFNAMES(user, *this) ;

}
