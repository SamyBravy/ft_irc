/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 16:37:41 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 00:47:02 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"

struct Client
{
	int fd;
    bool authenticated;
    std::string nickname;
    std::string username;
    std::string realname;
    
    Client(int clientFd = -1);
    ~Client();
};

#endif
