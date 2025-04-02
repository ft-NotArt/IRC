/* Includes */

#include "Channel.hpp"
#include "Errors.hpp"


/* Constructor */

Channel::Channel(const std::string &name, const Server &server, const User *creator) : server(server), name(name) {
	if (this->name.empty()
	||	this->name[0] != '#'
	||	this->name.find(' ') != std::string::npos
	||	this->name.find('\a') != std::string::npos
	||	this->name.find(',') != std::string::npos)
		throw IrcException::BadChanName(this->name) ;

	this->password = "" ;
	this->topic = "" ;
	this->topic_change.first = NULL ;
	this->topic_change.second = std::time(NULL) ;
	this->max_users = -1 ;
	this->invite_only = false ;
	this->topic_restrict = false ;

	this->join(creator, "") ;
	this->perms[creator] = OPERATOR ;
}


/* Get */

/***
 * @brief Used for RPL_NAMREPLY
 * @return A string containing the names of every members of the channel (with @ before the name for operators)
 */
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

bool	Channel::isUserIn(const User *user) const {
	if (this->users.find(user) != this->users.end())
		return true ;
	
	return false ;
}


/* Methods */

void Channel::join(const User *user, const std::string &password) {
	try {
		if (this->perms.at(user) & BANNED)
			throw(IrcException::BannedFromChan());
	} catch(const IrcException::BannedFromChan& e) {
		throw e ;
	} catch(const std::exception& e) {}

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
		this->perms.at(user) &= ~INVITED ; // If user is known as invited, removes its invited status
	} catch(const std::exception& e) {
		this->perms[user] = NORMAL ;
	}

	this->server.MSG_JOIN(user, *this) ;

	this->sendTopic(user) ;

	this->server.RPL_NAMREPLY(user, *this) ;
	this->server.RPL_ENDOFNAMES(user, *this) ;
}

void	Channel::leave(const User *user, const std::string &msg) {
	if (this->perms.find(user) != this->perms.end())
		this->perms.erase(user) ;

	if (!this->isUserIn(user))
		throw IrcException::NotOnChannel(this->getName()) ;

	this->users.erase(user) ;
	this->topic_change.first = NULL ;

	this->sendMsg(NULL, msg) ;
}

void	Channel::changeTopic(const User *user, const std::string &topic) {
	if (!this->isUserIn(user))
		throw IrcException::NotOnChannel(this->getName()) ;
	
	this->topic = topic ;
	this->topic_change.first = user ;
	this->topic_change.second = std::time(NULL) ;

	this->server.MSG_TOPIC(user, *this, topic) ;
}

void	Channel::sendTopic(const User *user) {
	if (this->topic != "") {
		this->server.RPL_TOPIC(user, *this) ;
		this->server.RPL_TOPICWHOTIME(user, *this) ;
	} else
		this->server.RPL_NOTOPIC(user, *this) ;
}

void	Channel::sendMsg(const User *user, const std::string &text) const {
	if (user && !this->isUserIn(user)) // Make sure user is in the channel, doesn't check if user == NULL (usefull for leave)
		throw IrcException::CannotSendToChan(this->getName()) ;
	
	for (std::set<const User *>::iterator it = this->users.begin(); it != this->users.end(); it++) {
		this->server.sendMsg((*it)->getFd(), text) ;
	}
}
