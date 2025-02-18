/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 15:07:14 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/18 16:49:44 by sdell-er         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "Server.hpp"

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Error: usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}
	if (!isDigit(argv[1]))
	{
		std::cerr << "Error: port must be a natural number" << std::endl;
		return 1;
	}

	Server server(strToNum<int>(argv[1]));

	try
	{
		server.run();
	}
	catch (const Server::ServerException &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	struct pollfd *fds = server.getPollfds();

	while (1)
	{
		int ret = poll(fds, MAX_CLIENTS + 1, -1);
		if (ret > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				int client_fd = accept(server.getFd(), NULL, NULL);
				if (client_fd != -1)
				{
					for (int i = 1; i <= MAX_CLIENTS; i++)
					{
						if (fds[i].fd == -1)
						{
							server.setClientFd(i, client_fd);
							std::cout << "New client connected (fd: " << client_fd << ")" << std::endl;
							break;
						}
					}
				}
			}
			
			for (int i = 1; i <= MAX_CLIENTS; i++)
			{
				if (fds[i].fd != -1 && (fds[i].revents & POLLIN))
				{
					char buffer[1024];
					int n = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (n > 0)
					{
						buffer[n] = '\0';
						std::cout << "Client " << fds[i].fd << " says: \"" << buffer << "\"" << std::endl;
						send(fds[i].fd, buffer, n, 0);
					}
					else if (n == 0)
					{
						std::cout << "Client " << fds[i].fd << " disconnected" << std::endl;
						close(fds[i].fd);
						fds[i].fd = -1;
					}
					else
					{
						std::cerr << "Error receiving data from client " << fds[i].fd << std::endl;
						close(fds[i].fd);
						fds[i].fd = -1;
					}
				}
			}
		}
		else if (ret == -1)
		{
			std::cerr << "Error polling sockets" << std::endl;
			break;
		}
	}

	return 0;
}

bool isDigit(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

