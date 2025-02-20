/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:11:19 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 23:47:20 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# define MAX_CLIENTS 10
# define MAX_CHANNELS 1024

# define PREFIX_ERR_NEEDMOREPARAMS ":ft_irc 461 "
# define ERR_NEEDMOREPARAMS " :Not enough parameters"
# define PREFIX_ERR_ALREADYREGISTRED ":ft_irc 462 "
# define ERR_ALREADYREGISTRED " :You may not reregister"
# define PREFIX_ERR_NONICKNAMEGIVEN ":ft_irc 431 "
# define ERR_NONICKNAMEGIVEN " :No nickname given"
# define PREFIX_ERR_NICKNAMEINUSE ":ft_irc 433 "
# define ERR_NICKNAMEINUSE " :Nickname is already in use"
# define PREFIX_ERR_PASSWDMISMATCH ":ft_irc 464 "
# define ERR_PASSWDMISMATCH " :Password incorrect"

# include <cstring>
# include <iostream>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <algorithm>
# include <sstream>
# include <poll.h>
# include <cstdlib>
# include <vector>
# include <fstream>

bool isDigit(const std::string &str);
std::vector<std::string> split(const std::string &str, char delim);
bool isStartSubstring(const std::string &str, const std::string &substr);
void sendMsg(int fd, std::string msg);
void printLog(const std::string &filename, std::string msg);
int countWords(const std::string &str);
std::string getWord(const std::string &str, int n);

template <typename T>
T strToNum(const std::string &str)
{
	std::stringstream ss(str);
	T num;

	ss >> num;

	return num;
}

#endif
