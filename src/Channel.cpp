/* Includes */

#include "Channel.hpp"
#include "Errors.hpp"


/* Constructor */

Channel::Channel(const std::string &name, const Server &server) : server(server), name(name) {
	this->password = "" ;
	this->topic = "" ;
	this->topic_change.first = NULL ;
	this->topic_change.second = std::time(NULL) ;
	this->max_users = -1 ;
	this->invite_only = false ;
	this->topic_restrict = false ;
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

// TODO Find a way to replace exception messages with the appropriated data (this is not easy as we thinked at the beginnind LOLLLLLLLLLLLLL)
void Channel::join(const User *user, const std::string &password) {

	if (this->perms.at(user) & BANNED)
	{
		throw(IrcException::BannedFromChan());
	}

	if (this->invite_only) {
		try {
			// if (!(this->perms.at(user) & INVITED))
			// 	; // return
		} catch(const std::exception& e) {
			throw(IrcException::InviteOnlyChan());
		}
	}

	if (this->max_users != -1 && (int) this->users.size() == this->max_users) {
		throw(IrcException::ChannelIsFull());
	}

	if (this->password != "" && password != this->password) {
		throw(IrcException::PasswdMismatch());
	}

	this->users.insert(user) ;
	try {
		this->perms.at(user) &= ~INVITED ;
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

void	Channel::sendMsg(const User *user, const std::string &text) {
	if (this->users.find(user) == this->users.end())
		throw IrcException::CannotSendToChan(this->getName()) ;
	
	(void) text ;
	// TODO: Send the text...
}
