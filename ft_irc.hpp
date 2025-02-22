#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# define MAX_CLIENTS 10
# define MAX_CHANNELS 1024

# define PREFIX_ERR_CUSTOM ":ft_irc 400 "
# define PREFIX_ERR_UNKNOWNCOMMAND ":ft_irc 421 "
# define ERR_UNKNOWNCOMMAND " :Unknown command"
# define PREFIX_ERR_NONICKNAMEGIVEN ":ft_irc 431 "
# define ERR_NONICKNAMEGIVEN " :No nickname given"
# define PREFIX_ERR_ERRONEUSNICKNAME ":ft_irc 432 "
# define ERR_ERRONEUSNICKNAME " :Erroneous nickname"
# define PREFIX_ERR_NICKNAMEINUSE ":ft_irc 433 "
# define ERR_NICKNAMEINUSE " :Nickname is already in use"
# define PREFIX_ERR_NEEDMOREPARAMS ":ft_irc 461 "
# define ERR_NEEDMOREPARAMS " :Not enough parameters"
# define PREFIX_ERR_ALREADYREGISTRED ":ft_irc 462 "
# define ERR_ALREADYREGISTRED " :You may not reregister"
# define PREFIX_ERR_PASSWDMISMATCH ":ft_irc 464 "
# define ERR_PASSWDMISMATCH " :Password incorrect"

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
# include <fstream>

std::vector<std::string> split(const std::string &str, char delim);
void printLog(const std::string &filename, std::string msg);
int countWords(const std::string &str);
std::string getWord(const std::string &str, int n);
std::string getDay(time_t t);
std::string getDate(time_t t);
std::string getTime(time_t t);

template <typename T>
T strToNum(const std::string &str)
{
	std::stringstream ss(str);
	T num;

	ss >> num;

	return num;
}

#endif
