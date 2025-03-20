#include "Server.hpp"
#include "Command.hpp"

// That should looks like this
void Command::QUIT(const User *client, std::vector<std::string> args) {
	std::string reason("") ;

	if (!args.empty())
		reason += args.at(0) ;

	// TODO quit every channels
}
