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
        std::string _ip;
		std::string _password;
		std::vector<Client> _clients;
		std::vector<Channel> _channels;
        time_t _creationMoment;

		
        void addClient(int client_fd);
        void removeClient(int i);
        bool clientExists(const std::string &nickname) const;
        struct pollfd *getPollfds() const;

		void createSocket();
		void bindServer();
		void listenServer();

        void sendMsg(int fd, std::string msg) const;
        void handleMessage(std::string message, int i);
        void passCommand(const std::string &message, Client &client);
        void nickCommand(const std::string &message, Client &client);
        void userCommand(const std::string &message, Client &client);
        void pingCommand(const std::string &message, Client &client);

	public:
		Server(int port = 8080, const std::string &password = "password");
		~Server();

		void run();
        void listenClients();
		
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
