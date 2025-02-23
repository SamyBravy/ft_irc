#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# define MAX_CLIENTS 10
# define MAX_CHANNELS 1024

# define PREFIX_ERR_CUSTOM ":ft_irc 400 "
# define PREFIX_ERR_NOSUCHNICK ":ft_irc 401 "
# define ERR_NOSUCHNICK " :No such nick/channel"
# define PREFIX_ERR_NOSUCHCHANNEL ":ft_irc 403 "
# define ERR_NOSUCHCHANNEL " :No such channel"
# define PREFIX_ERR_CANNOTSENDTOCHAN ":ft_irc 404 "
# define ERR_CANNOTSENDTOCHAN " :Cannot send to channel"
# define PREFIX_ERR_NORECIPIENT ":ft_irc 411 "
# define ERR_NORECIPIENT " :No recipient given (PRIVMSG)"
# define PREFIX_ERR_NOTEXTTOSEND ":ft_irc 412 "
# define ERR_NOTEXTTOSEND " :No text to send"
# define PREFIX_ERR_UNKNOWNCOMMAND ":ft_irc 421 "
# define ERR_UNKNOWNCOMMAND " :Unknown command"
# define PREFIX_ERR_NONICKNAMEGIVEN ":ft_irc 431 "
# define ERR_NONICKNAMEGIVEN " :No nickname given"
# define PREFIX_ERR_ERRONEUSNICKNAME ":ft_irc 432 "
# define ERR_ERRONEUSNICKNAME " :Erroneous nickname"
# define PREFIX_ERR_NICKNAMEINUSE ":ft_irc 433 "
# define ERR_NICKNAMEINUSE " :Nickname is already in use"
# define PREFIX_ERR_USERNOTINCHANNEL ":ft_irc 441 "
# define ERR_USERNOTINCHANNEL " :They aren't on that channel"
# define PREFIX_ERR_NOTONCHANNEL ":ft_irc 442 "
# define ERR_NOTONCHANNEL " :You're not on that channel"
# define PREFIX_ERR_USERONCHANNEL ":ft_irc 443 "
# define ERR_USERONCHANNEL " :is already on channel"
# define PREFIX_ERR_NEEDMOREPARAMS ":ft_irc 461 "
# define ERR_NEEDMOREPARAMS " :Not enough parameters"
# define PREFIX_ERR_ALREADYREGISTRED ":ft_irc 462 "
# define ERR_ALREADYREGISTRED " :You may not reregister"
# define PREFIX_ERR_PASSWDMISMATCH ":ft_irc 464 "
# define ERR_PASSWDMISMATCH " :Password incorrect"
# define PREFIX_ERR_CHANNELISFULL ":ft_irc 471 "
# define ERR_CHANNELISFULL " :Cannot join channel (+l)"
# define PREFIX_ERR_INVITEONLYCHAN ":ft_irc 473 "
# define ERR_INVITEONLYCHAN " :Cannot join channel (+i)"
# define PREFIX_ERR_BADCHANNELKEY ":ft_irc 475 "
# define ERR_BADCHANNELKEY " :Cannot join channel (+k)"
# define PREFIX_ERR_CHANOPRIVSNEEDED ":ft_irc 482 "
# define ERR_CHANOPRIVSNEEDED " :You're not channel operator"

# include <cstring>
# include <iostream>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <unistd.h>
# include <algorithm>
# include <sstream>
# include <poll.h>
# include <cstdlib>
# include <vector>
# include <map>
# include <set>
# include <fstream>

std::vector<std::string> split(const std::string &str, char delim);
int countWords(const std::string &str);
std::string getWord(const std::string &str, int n);
std::string getDay(time_t t);
std::string getDate(time_t t);
std::string getTime(time_t t);
std::string getMoment(time_t t);

template <typename T>
T strToNum(const std::string &str)
{
	std::stringstream ss(str);
	T num;

	ss >> num;

	return num;
}

#endif
