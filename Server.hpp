/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:10:56 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 12:10:57 by sdell-er         ###   ########.fr       */
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
		std::string _password;
		std::vector<Client> _clients;
		std::vector<Channel> _channels;
		
		void createSocket();
		void bindServer();
		void listenServer();
		
	public:
		Server(int port = 8080, std::string password = "password");
		~Server();

		int getFd() const { return _fd; }
		std::string getPassword() const { return _password; }
		int getClientsSize() const { return _clients.size(); }
		struct pollfd *getPollfds();
		
		void addClient(int client_fd);
		void removeClient(int i);

		void run();
		void handleMessage(std::string buffer, int i);
		
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
