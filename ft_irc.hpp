/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 23:10:08 by samuele           #+#    #+#             */
/*   Updated: 2025/02/18 16:40:54 by sdell-er         ###   ########.fr       */
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

template <typename T>
T strToNum(std::string str)
{
    std::stringstream ss(str);
    T num;

    ss >> num;

    return num;
}

#endif
