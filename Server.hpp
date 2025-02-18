/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 15:27:29 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/18 16:49:25 by sdell-er         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "Channel.hpp"
# include "Client.hpp"
# include "ft_irc.hpp"
class Server
{	
	private:
		int _fd;
		int _port;
		std::vector<Channel> _channels;
		std::vector<Client> _clients;
		
		void createSocket();
		void bindServer();
		void listenServer();
		
	public:
		Server(int port = 8080);
		~Server();

		int getFd() const { return _fd; }
		struct pollfd *getPollfds();
		void setClientFd(int i, int fd) { _clients[i]._fd = fd; }

		void run();
		class ServerException : public std::exception
		{
			private:
				const char *_msg;
			
			public:
				ServerException(const char *msg);
				virtual const char *what() const throw();
		};
};

#endif
