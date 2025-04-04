#include "Server.hpp"

/* Temp */
#define GREEN "\e[1;32m"
#define LIGHT_GREEN "\e[1;92m"
#define LIGHT_RED "\e[1;91m"
#define BLUE "\e[1;34m"
#define GRAY "\e[1;90m"
#define YELLOW "\e[1;93m"
#define CYAN "\e[1;96m"
#define RESET "\e[0m"

#include <sstream>

/* End temp */


/* CAP */

void	Server::handleCAP(std::stringstream &ssMessage, User *user) {
	std::string capName;
	ssMessage >> capName;

	if (capName == MSG_CLI_CAP_LS) {
		user->setInCAP(true) ;
		this->MSG_CAP_LS(user);
	}
	else if (capName == MSG_CLI_CAP_REQ) {
		std::string requested_cap ;
		std::getline(ssMessage, requested_cap) ;

		if (requested_cap.find(SERVER_CAP))
			this->MSG_CAP_ACK(user, SERVER_CAP) ;
	}
	else if (capName == MSG_CLI_CAP_END) {
		user->setInCAP(false) ;
		if (!user->getUsername().empty()) {
			user->setRegistered(true) ;
			this->greetings(user) ;
		}
	}
}

/* PASS */

void	Server::handlePASS(std::stringstream &ssMessage, User *user) {
	try {
		std::string password ;
		std::getline(ssMessage, password) ;
		password = trim(password) ;
		
		if (password.empty())
			throw IrcException::NeedMoreParams() ;
		else if (user->isRegistered())
			throw IrcException::AlreadyRegistered() ;
		
		/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << user->getFd() << "]] Password: `" << password << "`" << std::endl;
		user->setPassword(password) ;
	} CATCH_CMD(PASS)
}

/* NICK */

void	Server::handleNICK(std::stringstream &ssMessage, User *user) {
	try {
		std::string nick ;
		std::getline(ssMessage, nick) ;
		nick = trim(nick) ;

		if (nick.empty())
			throw IrcException::NoNicknameGiven() ;
		else if (nick[0] == '#' || nick.find(' ') != std::string::npos || nick.find(':') != std::string::npos)
			throw IrcException::ErroneusNickname(nick) ;
		else if (this->getUser(nick))
			throw IrcException::NicknameInUse(nick) ;
		
		/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << user->getFd() << "]] Nick: `" << nick << "`" << std::endl;
		user->setNickname(nick) ;
	} CATCH_CMD(NICK)
}

/* USER */

void	Server::handleUSER(std::stringstream &ssMessage, User *user) {
	try {
		std::string username;
		ssMessage >> username;

		if (username.empty())
			throw IrcException::NeedMoreParams() ;
		else if (user->isRegistered() || !user->getUsername().empty())
			throw IrcException::AlreadyRegistered() ;
		else if (user->getPassword() != this->password)
			throw IrcException::PasswdMismatch() ;
		else if (user->getNickname().empty())
			throw Server::DisconnectClient("Nickname error (invalid/empty)") ;

		// /* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << fd << "]] User Old: `" << message.substr(std::strlen(MSG_CLI_USER) + 1, message.find(' ', std::strlen(MSG_CLI_USER) + 1)) << "`" << std::endl;
		/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << user->getFd() << "]] User: `" << username << "`" << std::endl;

		user->setUsername(username) ;

		/* DEBUG */ std::cout << LIGHT_GREEN << "[DBUG|CLI[" << user->getFd() << "]] Client " << user->getFd() << " authenticated successfully." << "\e[0m" << std::endl;

		if (!user->isInCAP()) {
			user->setRegistered(true) ;
			this->greetings(user) ;
		}
	}
	catch(const Server::DisconnectClient &e) {
		throw e ;
	} catch(const IrcException::PasswdMismatch &e) {
		throw Server::DisconnectClient("Wrong Password") ;
	} CATCH_CMD(USER)
	
}

/* PING */

void	Server::handlePING(std::stringstream &ssMessage, User *user) {
	std::string token;
	ssMessage >> token;

	/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << user->getFd() << "]] PING: Token: `" << token << "`" << std::endl;
	this->MSG_PONG(user, token) ;
}

/* QUIT */

void	Server::handleQUIT(std::stringstream &ssMessage, User *user) {
	std::size_t colon_pos = ssMessage.str().find(':') ;

	std::string reason ;
	try {
		reason = trim(ssMessage.str().substr(colon_pos + 1));
	} catch (std::out_of_range &e) {
		reason = "no reason" ;
	}

	/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << user->getFd() << "]] QUIT: Reason: `" << reason << "`" << std::endl;
	this->QUIT(user, reason, true) ;
}

void	Server::QUIT(const User *client, const std::string &reason, bool requested) {
	this->MSG_ERROR(client, reason) ;

	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " QUIT :" ;
	if (requested)
		msg += "Quit: " ;
	msg += reason ;

	for (std::map<std::string, Channel *>::iterator it = this->channels.begin(); it != this->channels.end(); ) {
		try {
			it->second->leave(client, msg) ;
			if (it->second->getUsersNb() == 0) {
				delete it->second ;
				this->channels.erase(it++) ;
			} else {
				it++ ;
			}
		} catch (const std::exception& e) {
			it++ ;
		}
	}

	this->closeClient(client->getFd()) ;
	delete client ;
}

/* PRIVMSG */

void	Server::handlePRIVMSG(std::stringstream &ssMessage, User *user) {
	try {
		std::string tmp ;
		std::getline(ssMessage, tmp, ':') ;
		if (tmp.empty())
			throw IrcException::NoRecipient() ;

		std::stringstream ssTargets(trim(tmp)) ;
		std::vector<std::string> targets ;
		while (std::getline(ssTargets, tmp, ','))
			targets.push_back(tmp) ;
		if (targets.empty())
			throw IrcException::NoRecipient() ;

		/* DEBUG */ std::cout << YELLOW << "[DBUG|CLI[" << user->getFd() << "]] PRIVMSG: Targets: ";
		/* DEBUG */ for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
		/* DEBUG */ 	std::cout << "`" << *it << "`";
		/* DEBUG */ 	if (it != targets.end() - 1)
		/* DEBUG */ 		std::cout << ", ";
		/* DEBUG */ }
		/* DEBUG */ std::cout << RESET << std::endl;

		std::getline(ssMessage, tmp);
		tmp = trim(tmp);
		if (tmp.empty())
			throw IrcException::NoTextToSend() ;

		/* DEBUG */ std::cout << YELLOW "[DBUG|CLI[" << user->getFd() << "]] PRIVMSG: Text: `" << tmp << "`" << RESET << std::endl;

		this->PRIVMSG(user, targets, tmp) ;
	} CATCH_CMD(PRIVMSG)
}

void	Server::PRIVMSG(const User *client, const std::vector<std::string> targets, std::string text) {
	for (std::vector<std::string>::const_iterator it = targets.begin(); it != targets.end(); it++) {
		if ((*it)[0] == '#') {
			if (!this->getChannel(*it))
				throw IrcException::NoSuchChannel(*it) ;
		} else {
			if (!this->getUser(*it))
				throw IrcException::NoSuchNick(*it) ;
		}
	}

	for (std::vector<std::string>::const_iterator it = targets.begin(); it != targets.end(); it++) {
		std::string msg(":") ;
		msg += client->getNickname() ;
		msg += " PRIVMSG " ;
		msg += (*it) ;
		msg += " :" ;
		msg += text ;

		if ((*it)[0] == '#') {
			this->getChannel(*it)->sendMsg(client, msg) ;
		} else {
			this->sendMsg(this->getUser(*it)->getFd(), msg) ;
		}
	}
}

/* JOIN */

void	Server::handleJOIN(std::stringstream &ssMessage, User *user) {
	try {
		std::string tmp ;

		ssMessage >> tmp ;
		if (tmp.empty())
			throw IrcException::NeedMoreParams() ;
		std::stringstream channels(tmp) ;

		ssMessage >> tmp ;
		std::stringstream keys(tmp) ;

		std::string channel ;
		std::string key ;
		while (std::getline(channels, channel, ',')) {
			std::getline(keys, key, ',') ;
			try {
				if (channel[0] != '#')
					throw IrcException::BadChanMask(channel) ;
				this->JOIN(user, channel, key) ;
			} CATCH_CMD(JOIN)
			key.clear() ; // Reset to ensure we don't keep the key from before
		}
	} CATCH_CMD(JOIN)
}

void	Server::JOIN(const User *client, const std::string &channel, const std::string &key) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan) { // Creating the channel (making the client operator)
		Channel *newChan = new Channel(channel, *this, client) ;
		this->channels.insert(std::pair<std::string, Channel *>(channel, newChan)) ;
	} else {
		chan->join(client, key) ;
	}
}

/* INVITE */

void	Server::handleINVITE(std::stringstream &ssMessage, User *user) {
	try {
		std::string nickname, channel ;
		ssMessage >> nickname ;
		ssMessage >> channel ;

		if (nickname.empty() || channel.empty())
			throw IrcException::NeedMoreParams() ;
		if (channel[0] != '#')
			throw IrcException::BadChanMask(channel) ;
						
		this->INVITE(user, nickname, channel) ;
	} CATCH_CMD(INVITE)
}

void	Server::INVITE(const User *client, const std::string &nickname, const std::string &channel) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;
	
	User *invited = this->getUser(nickname) ;
	if (!invited)
		throw IrcException::NoSuchNick(nickname) ;
	
	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " INVITE " ;
	msg += nickname ;
	msg += " " ;
	msg += channel ;

	chan->invite(client, invited) ;
	this->RPL_INVITING(client, invited, *chan) ;
	this->sendMsg(invited->getFd(), msg) ;
}

/* PART */

void	Server::handlePART(std::stringstream &ssMessage, User *user) {
	try {
		std::string strChannels;
		std::getline(ssMessage, strChannels, ':') ;
		strChannels = trim(strChannels) ;
		if (strChannels.empty())
			throw IrcException::NeedMoreParams() ;

		std::string reason;
		std::getline(ssMessage, reason) ;
		reason = trim(reason) ;

		std::stringstream ssChannels(strChannels) ;
		std::string channel ;
		while (std::getline(ssChannels, channel, ',')) {
			try {
				if (channel[0] != '#')
					throw IrcException::BadChanMask(channel) ;

				this->PART(user, channel, reason) ;
			} CATCH_CMD(PART)
		}
	} CATCH_CMD(PART)
}

void	Server::PART(const User *client, const std::string &channel, const std::string &reason) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;
	if (!chan->isUserIn(client))
		throw IrcException::NotOnChannel(channel) ;

	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " PART " ;
	msg += chan->getName() ;
	if (reason != "") {
		msg += " :" ;
		msg += reason ;
	}

	chan->leave(client, msg) ;
	this->sendMsg(client->getFd(), msg) ;

	if (chan->getUsersNb() == 0) {
		delete chan ;
		this->channels.erase(channel) ;
	}
}

/* KICK */

void	Server::handleKICK(std::stringstream &ssMessage, User *user) {
	try {
		std::string tmp;
		std::getline(ssMessage, tmp, ':') ;
		tmp = trim(tmp) ;

		std::stringstream ssTmp(tmp) ;
		std::string channel ;
		ssTmp >> channel ;
		if (channel.empty())
			throw IrcException::NeedMoreParams() ;

		ssTmp >> tmp ;
		if (tmp.empty())
			throw IrcException::NeedMoreParams() ;

		if (channel[0] != '#')
			throw IrcException::BadChanMask(channel) ;

		std::string comment ;
		std::getline(ssMessage, comment) ;
		comment = trim(comment) ;
		if (comment.empty())
			comment = "pas sage" ;

		ssTmp.str(tmp) ;
		while (std::getline(ssTmp, tmp, ',')) {
			try {
				this->KICK(user, channel, tmp, comment) ;
			} CATCH_CMD(KICK)
		}
	} CATCH_CMD(KICK)
}

void	Server::KICK(const User *client, const std::string &channel, const std::string &kickedUser, const std::string &comment) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;

	if (!chan->isUserIn(client))
		throw IrcException::NotOnChannel(channel) ;

	User *kicked = this->getUser(kickedUser) ;
	if (!kicked)
		throw IrcException::NoSuchNick(kickedUser) ;

	std::string msg(":") ;
	msg += client->getNickname() ;
	msg += " KICK " ;
	msg += chan->getName() ;
	msg += " " ;
	msg += kickedUser ;
	msg += " :" ;
	msg += comment ;

	chan->kick(client, kicked, msg) ;
}

/* TOPIC */

void	Server::handleTOPIC(std::stringstream &ssMessage, User *user) {
	try {
		std::string	channel;
		std::getline(ssMessage, channel, ':') ;
		channel = trim(channel);

		if (channel.empty())
			throw IrcException::NeedMoreParams() ;
		else if (channel[0] != '#')
			throw IrcException::BadChanMask(channel) ;
		else if (!this->getChannel(channel))
			throw IrcException::NoSuchChannel(channel) ;

		std::string	topic;
		std::getline(ssMessage, topic) ;
		topic = trim(topic);

		this->TOPIC(user, channel, topic, !topic.empty()) ;
	} CATCH_CMD(TOPIC)
}

void	Server::TOPIC(const User *client, const std::string &channel, const std::string &topic, bool modify) {
	Channel *chan = this->getChannel(channel) ;
	if (!chan)
		throw IrcException::NoSuchChannel(channel) ;

	if (modify) {
		chan->changeTopic(client, topic) ;
	} else {
		chan->sendTopic(client) ;
	}
}

/* MODE */

// Find target fd by name and let know the server if target is already in channel and if user does exist maybe in the order
void Server::MODE(const User *client, const std::string &channel, const std::vector<std::string> &modesArgs)
{
	if (!this->getChannel(channel))
		throw IrcException::NoSuchNick(channel);

	Channel *chan = this->getChannel(channel);

	if (modesArgs.empty())
	{
		this->RPL_CHANNELMODEIS(client, chan->getName(), chan->getModesString(), chan->getModesArgs());
		return ;
	}

	bool		inAdditionSign = true;
	size_t		argsIndex = 1;

	for (size_t i = 0; i < modesArgs.at(0).size(); i++)
	{
		if (modesArgs.at(0)[i] == '-')
		{
			inAdditionSign = false;
			i++;
		}
		else if (modesArgs.at(0)[i] == '+')
		{
			inAdditionSign = true;
			i++;
		}

		if (!chan->isUserIn(client))
			throw IrcException::NotOnChannel(chan->getName()) ;
		if (!chan->hasPerms(client, OPERATOR))
			throw IrcException::ChanoPrivNeeded(chan->getName()) ;

		switch (modesArgs.at(0)[i])
		{
			case 'i':
				chan->setInviteOnly(inAdditionSign);
				break;
			case 't':
				chan->setTopicRestrict(inAdditionSign);
				break;
			case 'k':
				if (inAdditionSign)
					chan->setPassword(modesArgs.at(argsIndex));
				else
					chan->setPassword("");
				break;
			case 'o':
				if (inAdditionSign)
					chan->addPerms(client, OPERATOR);
				else
					chan->removePerms(client, OPERATOR);
				break;
			case 'l':
				if (inAdditionSign)
					chan->setMaxUsers(atoi(modesArgs.at(argsIndex).c_str()));
				else
					chan->setMaxUsers(-1);
				break;
			default:
				throw IrcException::UnknownMode();
		}
		if (modesArgs.size() > 1)
			chan->setModesArgs(modesArgs.at(argsIndex) + " ");
		argsIndex++;
	}
	chan->setModesString(modesArgs.at(0));
}
