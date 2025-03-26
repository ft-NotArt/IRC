/* Includes */

#include "User.hpp"


/* Constructor */

User::User(int fd) : fd(fd), authenticated(false), requestCap(false) {}
