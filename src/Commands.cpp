#include "Server.hpp"
#include "Errors.hpp"

// That should looks like this
void Server::QUIT(const User *client, std::vector<std::string> args) {
	std::string reason("") ;

	if (!args.empty())
		reason += args.at(0) ;

	// TODO quit every channels
}

void Server::INVITE(const User *client, std::vector<std::string> args)
{
	if (args.size() < 2)
	{
		throw(IrcException::NeedMoreParams());
	}

	std::string target = args.at(0);
	std::string channel = args.at(1);

	if (channels.find(channel) == channels.end())
	{
		throw(IrcException::NoSuchChannel());
	}

	// Check if client is in the channel or not ?

	// Find target fd by name and let know the server if target is already in channel and if user does exist maybe in the order
}
