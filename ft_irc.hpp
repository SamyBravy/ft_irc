/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 23:10:08 by samuele           #+#    #+#             */
/*   Updated: 2025/02/19 00:31:48 by samuele          ###   ########.fr       */
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
