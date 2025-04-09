#pragma once

#include <stdexcept>

class IrcException {
	public:
		/// @brief Indicates that no client can be found for the supplied nickname. The text used in the last param of this message may vary.
		class NoSuchNick : public std::exception {
			private:
				std::string message ;
			public:
				NoSuchNick(std::string nick) {
					this->message = ":Internet_Relay_Chat 401 %client% " + nick + " :No such nick/channel" ;
				}
				virtual ~NoSuchNick() throw() {}

				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Indicates that no channel can be found for the supplied channel name. The text used in the last param of this message may vary.
		class NoSuchChannel : public std::exception {
			private:
				std::string message ;
			public:
				NoSuchChannel(std::string channel) {
					this->message = ":Internet_Relay_Chat 403 %client% " + channel + " :No such channel" ;
				}
				virtual ~NoSuchChannel() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Indicates that the PRIVMSG / NOTICE could not be delivered to <channel>. The text used in the last param of this message may vary. This is generally sent in response to channel modes, such as a channel being moderated and the client not having permission to speak on the channel, or not being joined to a channel with the no external messages mode set.
		class CannotSendToChan : public std::exception {
			private:
				std::string message ;
			public:
				CannotSendToChan(std::string channel) {
					this->message = ":Internet_Relay_Chat 404 %client% " + channel + " :Cannot send to channel" ;
				}
				virtual ~CannotSendToChan() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned by the PRIVMSG command to indicate the message wasn’t delivered because there was no recipient given.
		class NoRecipient : public std::exception {
			public:
				virtual const char *what() const throw() { return ":Internet_Relay_Chat 411 %client% :No recipient given (%command%)"; };
		};

		/// @brief Returned by the PRIVMSG command to indicate the message wasn’t delivered because there was no text to send.
		class NoTextToSend : public std::exception {
			public:
				virtual const char *what() const throw() { return ":Internet_Relay_Chat 412 %client% :No text to send"; };
		};

		/// @brief Returned when a nickname parameter is expected for a command but isn’t given.
		class NoNicknameGiven : public std::exception {
			public:
				virtual const char *what() const throw() { return ":Internet_Relay_Chat 431 %client% :No nickname given"; };
		};

		/// @brief Returned when a NICK command cannot be successfully completed as the desired nickname contains characters that are disallowed by the server. See the NICK command for more information on characters which are allowed in various IRC servers. The text used in the last param of this message may vary.
		class ErroneusNickname : public std::exception {
			private:
				std::string message ;
			public:
				ErroneusNickname(std::string nick) {
					this->message = ":Internet_Relay_Chat 432 %client% " + nick + " :Erroneus nickname" ;
				}
				virtual ~ErroneusNickname() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned when a NICK command cannot be successfully completed as the desired nickname is already in use on the network. The text used in the last param of this message may vary.
		class NicknameInUse : public std::exception {
			private:
				std::string message ;
			public:
				NicknameInUse(std::string nick) {
					this->message = ":Internet_Relay_Chat 433 %client% " + nick + " :Nickname is already in use" ;
				}
				virtual ~NicknameInUse() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned when a client tries to perform a channel+nick affecting command, when the nick isn’t joined to the channel (for example, MODE #channel +o nick).
		class UserNotInChannel : public std::exception {
			private:
				std::string message ;
			public:
				UserNotInChannel(std::string nick, std::string channel) {
					this->message = ":Internet_Relay_Chat 441 %client% " + nick + " " + channel + " :They aren't on that channel" ;
				}
				virtual ~UserNotInChannel() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned when a client tries to perform a channel-affecting command on a channel which the client isn’t a part of.
		class NotOnChannel : public std::exception {
			private:
				std::string message ;
			public:
				NotOnChannel(std::string channel) {
					this->message = ":Internet_Relay_Chat 442 %client% " + channel + " :You're not on that channel" ;
				}
				virtual ~NotOnChannel() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned when a client tries to invite <nick> to a channel they’re already joined to.
		class UserOnChannel : public std::exception {
			private:
				std::string message ;
			public:
				UserOnChannel(std::string nick, std::string channel) {
					this->message = ":Internet_Relay_Chat 443 %client% " + nick + " " + channel + " :is already on channel" ;
				}
				virtual ~UserOnChannel() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned when a client command cannot be parsed as they are not yet registered. Servers offer only a limited subset of commands until clients are properly registered to the server. The text used in the last param of this message may vary.
		class NotRegistered : public std::exception {
			public:
				virtual const char *what() const throw() { return ":Internet_Relay_Chat 451 %client% :You have not registered"; };
		};

		/// @brief Returned when a client command cannot be parsed because not enough parameters were supplied. The text used in the last param of this message may vary.
		class NeedMoreParams : public std::exception {
			public:
				virtual const char *what() const throw() { return ":Internet_Relay_Chat 461 %client% %command% :Not enough parameters"; };
		};

		/// @brief Returned when a client tries to change a detail that can only be set during registration (such as resending the PASS or USER after registration). The text used in the last param of this message varies.
		class AlreadyRegistered : public std::exception {
			public:
				virtual const char *what() const throw() { return ":Internet_Relay_Chat 462 %client% :You may not reregister"; };
		};

		/// @brief Returned to indicate that the connection could not be registered as the password was either incorrect or not supplied. The text used in the last param of this message may vary.
		class PasswdMismatch : public std::exception {
			public:
				virtual const char *what() const throw() { return ":Internet_Relay_Chat 464 %client% :Password incorrect"; };
		};

		/// @brief Returned to indicate that a JOIN command failed because the client limit mode has been set and the maximum number of users are already joined to the channel. The text used in the last param of this message may vary.
		class ChannelIsFull : public std::exception {
			private:
				std::string message ;
			public:
				ChannelIsFull(std::string channel) {
					this->message = ":Internet_Relay_Chat 471 %client% " + channel + " :Cannot join channel (+l)" ;
				}
				virtual ~ChannelIsFull() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Indicates that a mode character used by a client is not recognized by the server. The text used in the last param of this message may vary.
		class UnknownMode : public std::exception {
			private:
				std::string message ;
			public:
				UnknownMode(std::string modechar) {
					this->message = ":Internet_Relay_Chat 472 %client% " + modechar + " :is unknown mode char to me" ;
				}
				virtual ~UnknownMode() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned to indicate that a JOIN command failed because the channel is set to [invite-only] mode and the client has not been invited to the channel or had an invite exception set for them. The text used in the last param of this message may vary.
		class InviteOnlyChan : public std::exception {
			private:
				std::string message ;
			public:
				InviteOnlyChan(std::string channel) {
					this->message = ":Internet_Relay_Chat 473 %client% " + channel + " :Cannot join channel (+i)" ;
				}
				virtual ~InviteOnlyChan() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned to indicate that a JOIN command failed because the client has been banned from the channel and has not had a ban exception set for them. The text used in the last param of this message may vary.
		class BannedFromChan : public std::exception {
			private:
				std::string message ;
			public:
				BannedFromChan(std::string channel) {
					this->message = ":Internet_Relay_Chat 474 %client% " + channel + " :Cannot join channel (+b)" ;
				}
				virtual ~BannedFromChan() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Returned to indicate that a JOIN command failed because the channel requires a key and the key was either incorrect or not supplied. The text used in the last param of this message may vary. Not to be confused with ERR_INVALIDKEY, which may be returned when setting a key.
		class BadChannelKey : public std::exception {
			private:
				std::string message ;
			public:
				BadChannelKey(std::string channel) {
					this->message = ":Internet_Relay_Chat 475 %client% " + channel + " :Cannot join channel (+k)" ;
				}
				virtual ~BadChannelKey() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Indicates the supplied channel name is not a valid. This is similar to, but stronger than, ERR_NOSUCHCHANNEL (403), which indicates that the channel does not exist, but that it may be a valid name. The text used in the last param of this message may vary.
		class BadChanMask : public std::exception {
			private:
				std::string message ;
			public:
				BadChanMask(std::string channel) {
					this->message = ":Internet_Relay_Chat 476 " + channel + " :Bad Channel Mask" ;
				}
				virtual ~BadChanMask() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Indicates the supplied channel name is not valid.
		class BadChanName : public std::exception {
			private:
				std::string message ;
			public:
				BadChanName(std::string channel) {
					this->message = ":Internet_Relay_Chat 479 %client% " + (channel.empty() ? "\"\"" : channel) + " :Illegal channel name" ;
				}
				virtual ~BadChanName() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Indicates that a command failed because the client does not have the appropriate channel privileges. This numeric can apply for different prefixes such as halfop, operator, etc. The text used in the last param of this message may vary.
		class ChanoPrivNeeded : public std::exception {
			private:
				std::string message ;
			public:
				ChanoPrivNeeded(std::string channel) {
					this->message = ":Internet_Relay_Chat 482 %client% " + channel + " :You're not channel operator" ;
				}
				virtual ~ChanoPrivNeeded() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};

		/// @brief Indicates that there was a problem with a mode parameter. Replaces various implementation-specific mode-specific numerics.
		class InvalidModeParam : public std::exception {
			private:
				std::string message ;
			public:
				InvalidModeParam(std::string channel, std::string modechar, std::string param) {
					this->message = ":Internet_Relay_Chat 696 %client% " + channel + " " + modechar + " " + param + " :Invalid mode parameter" ;
				}
				virtual ~InvalidModeParam() throw() {}
			
				virtual const char *what() const throw() { return this->message.c_str() ; };
		};
};

