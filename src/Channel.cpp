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
	this->modesString = "" ;
	this->modesArgs = "" ;
	this->topic_change.first = NULL ;
	this->topic_change.second = std::time(NULL) ;
	this->max_users = -1 ;
	this->invite_only = false ;
	this->topic_restrict = false ;

	this->join(creator, "") ;
	this->perms[creator] = OPERATOR ;
}


/* Get & Set */

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

bool	Channel::hasPerms(const User *user, uint8_t perms) {
	bool hasPerms = false;

	try {
		hasPerms = this->perms[user] & perms;
	} catch (const std::exception &ex) {}

	return hasPerms;
}

void	Channel::addPerms(const User *user, uint8_t perms) {
	try {
		this->perms[user] |= perms;
	} catch (const std::exception &ex) {
		this->perms[user] = perms;
	}
}

void	Channel::removePerms(const User *user, uint8_t perms) {
	try {
		this->perms[user] &= ~perms;
	} catch (const std::exception &ex) {}
}


/* Methods */

void Channel::join(const User *user, const std::string &password) {
	try {
		if (this->perms.at(user) & BANNED)
			throw(IrcException::BannedFromChan(this->name));
	} catch(const IrcException::BannedFromChan& e) { // This is relevant, don't change
		throw e ;
	} catch(const std::exception& e) {}

	if (this->invite_only) {
		try {
			if (!(this->perms.at(user) & INVITED))
				return ;
		} catch(const std::exception& e) {
			throw(IrcException::InviteOnlyChan(this->name));
		}
	}

	if (this->max_users != -1 && (int) this->users.size() >= this->max_users) {
		throw(IrcException::ChannelIsFull(this->name));
	}

	if (this->password != "" && password != this->password) {
		throw(IrcException::BadChannelKey(this->name));
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

void	Channel::ban(const User *user, const User *banned) {
	this->kick(user, banned, "") ;
	
	std::string msg(":") ;
	msg += user->getFullname() ;
	msg += " MODE " ;
	msg += this->name ;
	msg += " +b " ;
	msg += banned->getNickname() ;
	this->server.sendMsg(banned->getFd(), msg) ;

	this->perms[banned] = BANNED ;
}

void	Channel::kick(const User *user, const User *kicked, const std::string &msg) {
	if (!this->isUserIn(kicked))
		throw IrcException::UserNotInChannel(user->getNickname(), this->name) ;
	if (!(this->perms.at(user) & OPERATOR))
		throw IrcException::ChanoPrivNeeded(this->name) ;

	this->server.sendMsg(kicked->getFd(), msg) ;
	this->leave(kicked, msg) ;
}

void	Channel::leave(const User *user, const std::string &msg) {
	if (this->perms.find(user) != this->perms.end())
		this->perms.erase(user) ;

	if (!this->isUserIn(user))
		throw IrcException::NotOnChannel(this->getName()) ;

	this->users.erase(user) ;
	if (user == this->topic_change.first)
		this->topic_change.first = NULL ;

	this->sendMsg(NULL, msg) ;
}

void	Channel::invite(const User *user, const User *invited) {
	if (!this->isUserIn(user))
		throw IrcException::NotOnChannel(this->getName()) ;
	else if (this->isUserIn(invited))
		throw IrcException::UserOnChannel(invited->getNickname(), this->getName()) ;
	else if (this->invite_only && !(this->perms.at(user) & OPERATOR))
		throw IrcException::ChanoPrivNeeded(this->name) ;
	
	this->perms.insert(std::pair<const User *, uint8_t>(invited, INVITED)) ;
}

void	Channel::changeTopic(const User *user, const std::string &topic) {
	if (!this->isUserIn(user))
		throw IrcException::NotOnChannel(this->getName()) ;
	if (topic_restrict && !(this->perms.at(user) & OPERATOR))
		throw IrcException::ChanoPrivNeeded(this->name) ;

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
		if ((*it) != user)
			this->server.sendMsg((*it)->getFd(), text) ;
	}
}
