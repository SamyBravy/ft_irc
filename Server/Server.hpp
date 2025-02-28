#ifndef SERVER_HPP
# define SERVER_HPP

# include "../ft_irc.hpp"
# include "Channel.hpp"
# include "Client.hpp"

class Channel;
struct Client;

class Server
{	
	private:
		static int _fd;
		int _port;
        std::string _ip;
		std::string _password;
		std::vector<Client> _clients;
		std::map<std::string, Channel> _channels;
        time_t _creationMoment;

        void removeClient(Client *client);
        void removeEmptyChannels();
        std::vector<struct pollfd> getPollfds() const;
        
		void createSocket();
		void bindServer();
		void listenServer();
        Client &getClient(const std::string nickname);
        static void closeSocket(int signal);
        
        void sendWelcomeMessage(const Client &client) const;
        void handleMessage(std::string message, size_t i);

        void capCommand(const std::string &message, Client &client);
        void passCommand(const std::string &message, Client &client);
        void nickCommand(const std::string &message, Client &client);
        void userCommand(const std::string &message, Client &client);
        void pingCommand(const std::string &message, Client &client);
        void whoCommand(const std::string &message, Client &client);
        void joinCommand(const std::string &message, Client &client);
        void privmsgCommand(const std::string &message, Client &client);
        void noticeCommand(const std::string &message, Client &client);
        void modeCommand(const std::string &message, Client &client);
        void quitCommand(const std::string &message, Client &client);
        void partCommand(const std::string &message, Client &client);
        void kickCommand(const std::string &message, Client &client);
        void inviteCommand(const std::string &message, Client &client);
        void topicCommand(const std::string &message, Client &client);

        bool inviteOnlyMode(const std::string &channel, const std::string &argument, bool add, Client &client);
        bool topicProtectedMode(const std::string &channel, const std::string &argument, bool add, Client &client);
        bool passwordMode(const std::string &channel, const std::string &argument, bool add, Client &client);
        bool limitMode(const std::string &channel, const std::string &argument, bool add, Client &client);
        bool operatorMode(const std::string &channel, const std::string &argument, bool add, Client &client);

        std::map<std::string, void (Server::*)(const std::string&, Client&)> _commands;
        std::map<char, bool (Server::*)(const std::string&, const std::string&, bool, Client&)> _modes;
        
	public:
        Server(int port = 8080, const std::string &password = "password");
        ~Server();
    
        void initCommandsAndModes();
		void run();
        void listenClients();

        void sendMsg(int fd, std::string msg) const;
        
        bool clientExists(const std::string &nickname) const;
        bool channelExists(const std::string &channelName) const;
		
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
