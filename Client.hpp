/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:11:24 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 12:11:25 by sdell-er         ###   ########.fr       */
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
