/* Includes */

#include "Server.hpp"


/* Constructor */

Server::Server(const std::string &password, const int port) : password(password), port(port) {}


/* Methods */

void Server::addUser(int fd, const User *user) {
	this->users[fd] = user ;
}
