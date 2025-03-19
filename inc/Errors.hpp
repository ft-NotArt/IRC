#pragma once

#include <stdexcept>

class IrcException {
	public:
		class NoNicknameGivenException : public std::exception {
			virtual const char *what() const throw() { return ":IRC 431 %client% :No nickname given"; };
		};

		class ErroneusNicknameException : public std::exception {
			virtual const char *what() const throw() { return ":IRC 432 %client% %nick% :Erroneus nickname"; };
		};

		class NicknameInUseException : public std::exception {
			virtual const char *what() const throw() { return ":IRC 432 %client% %nick% :Nickname is already in use"; };
		};

		class UserNotInChannelException : public std::exception {
			virtual const char *what() const throw() { return ":IRC 441 %client% %nick% %channel% :They aren't on that channel"; };
		};

		class NotOnChannel : public std::exception {
			virtual const char *what() const throw() { return ":IRC 442 %client% %channel% :You're not on that channel"; };
		};

		class UserOnChannel : public std::exception {
			virtual const char *what() const throw() { return ":IRC 443 %client% %nick% %channel% :is already on channel"; };
		};

		class NotRegistered : public std::exception {
			virtual const char *what() const throw() { return ":IRC 451 %client% :You have not registered"; };
		};

		class NeedMoreParams : public std::exception {
			virtual const char *what() const throw() { return ":IRC 461 %client% %command% :Not enough parameters"; };
		};

		class AlreadyRegistered : public std::exception {
			virtual const char *what() const throw() { return ":IRC 462	 %client% :You may not reregister"; };
		};

		class PasswdMismatch : public std::exception {
			virtual const char *what() const throw() { return ":IRC 464	%client% :Password incorrect"; };
		};

		class YourReBannedCreep : public std::exception {
			virtual const char *what() const throw() { return ":IRC 465	%client% :You are banned from this server."; };
		};

		class ChannelIsFull : public std::exception {
			virtual const char *what() const throw() { return ":IRC 471	%client% %channel% :Cannot join channel (+l)"; };
		};

		class UnknownMode : public std::exception {
			virtual const char *what() const throw() { return ":IRC 472	%client% %modechar% :is unknown mode char to me"; };
		};

		class InviteOnlyChan : public std::exception {
			virtual const char *what() const throw() { return ":IRC 473	%client% %channel% :Cannot join channel (+i)"; };
		};

		class BannedFromChan : public std::exception {
			virtual const char *what() const throw() { return ":IRC 474	%client% %channel% :Cannot join channel (+b)"; };
		};

		class BadChannelKey : public std::exception {
			virtual const char *what() const throw() { return ":IRC 475	%client% %channel% :Cannot join channel (+k)"; };
		};

		class BadChanMask : public std::exception {
			virtual const char *what() const throw() { return ":IRC 476 %channel% :Bad Channel Mask"; };
		};

		class NoPrivileges : public std::exception {
			virtual const char *what() const throw() { return ":IRC 481 %client% :Permission Denied- You're not an IRC operator"; };
		};

		class ChanoPrivNeeded : public std::exception {
			virtual const char *what() const throw() { return ":IRC 482 %client% %channel% :You're not channel operator"; };
		};
};

