/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:11:24 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 23:52:11 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"

struct Client
{
	int fd;
	bool authenticated;
    short int authState;
    std::string password;
	std::string nickname;
	std::string username;
	std::string realname;
    std::string hostname;
    std::string servername;
	
	Client(int clientFd = -1);
	~Client();
};

#endif
