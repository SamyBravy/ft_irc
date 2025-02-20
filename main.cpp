/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:11:14 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 23:09:40 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "Server.hpp"

int main(int argc, char *argv[])
{
    remove("serverMessages.txt");
    remove("clientMessages.txt");

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

	Server server(strToNum<int>(argv[1]), argv[2]);

	try
	{
		server.run();
	}
	catch (const Server::ServerException &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	while (1)
	{
		struct pollfd *fds = server.getPollfds();
		
		int ret = poll(fds, MAX_CLIENTS + 1, -1);
		if (ret > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				int client_fd = accept(server.getFd(), NULL, NULL);
				if (client_fd != -1)
                {
                    try
                    {
                        server.addClient(client_fd);
                    }
                    catch (const Server::ServerException &e)
                    {
                        std::cerr << e.what() << std::endl;
                        close(client_fd);
                    }
                }
				else
					std::cerr << "Error accepting new client" << std::endl;
			}
			
			for (int i = 1; i <= server.getClientsSize() && fds[i].fd != -1; i++)
			{
				if (fds[i].revents & POLLIN)
				{
					char buffer[1024];
					int n = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
					if (n > 0)
					{
						buffer[n] = '\0';
                        printLog("clientMessages.txt", buffer);
						server.handleMessage(buffer, i - 1);
					}
					else if (n == 0)
					{
						std::cout << "Client " << fds[i].fd << " disconnected" << std::endl;
						server.removeClient(i - 1);
					}
					else
					{
						std::cerr << "Error receiving data from client " << fds[i].fd << std::endl;
						server.removeClient(i - 1);
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

bool isDigit(const std::string &str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

std::vector<std::string> split(const std::string &str, char delim)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delim))
    {
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        tokens.push_back(token);
    }

    return tokens;
}

bool isStartSubstring(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    return str.substr(0, substr.size()) == substr;
}

void printLog(const std::string &filename, std::string msg)
{
    std::ofstream file(filename.c_str(), std::ios_base::app);
    msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
    file << msg << std::endl;
    file.close();
}

void sendMsg(int fd, std::string msg)
{
    msg += "\r\n";
    if (send(fd, msg.c_str(), msg.size(), 0) == -1)
        std::cerr << "Error sending message" << std::endl;
    
    printLog("serverMessages.txt", msg);
}

int countWords(const std::string &str)
{
    std::stringstream ss(str);
    std::string word;
    int count = 0;

    while (ss >> word)
        count++;
        
    return count;
}

std::string getWord(const std::string &str, int n)
{
    std::stringstream ss(str);
    std::string word;
    int count = 0;

    while (ss >> word)
    {
        if (count == n)
            return word;
        count++;
    }    
    return "";
}
