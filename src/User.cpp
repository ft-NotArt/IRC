/* Includes */

#include "User.hpp"


/* Constructor */

User::User(int fd, const std::string &nickname, const std::string &username) : fd(fd), nickname(nickname), username(username) {}
