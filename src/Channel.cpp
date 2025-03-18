/* Includes */

#include "Channel.hpp"


/* Methods */

void Channel::addUser(const User *user, uint8_t perms) {
	this->users.insert(user) ;
	this->perms[user] = perms ;
}
