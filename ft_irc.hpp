/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:11:19 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 12:11:20 by sdell-er         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# define MAX_CLIENTS 10
# define MAX_CHANNELS 1024

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

bool isDigit(std::string str);
std::vector<std::string> split(const std::string &str, std::string delim);

template <typename T>
T strToNum(std::string str)
{
	std::stringstream ss(str);
	T num;

	ss >> num;

	return num;
}

#endif
