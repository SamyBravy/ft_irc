#ifndef BravyBOT_HPP
# define BravyBOT_HPP

# include "../ft_irc.hpp"

class BravyBot
{
    private:
        std::string _nickname;
        static int _socketFd;
        std::string _serverIp;
        int _serverPort;
        std::string _serverPassword;
        std::map<std::string, std::string> _commands;

        void connectToServer();
        void handleMessages(struct pollfd &pollFd);
        static void closeSocket(int signal);
        void sendMsg(std::string msg) const;
    
    public:
        BravyBot(const std::string &ip = "127.0.1.1", int port = 8080, const std::string &password = "password");
        ~BravyBot();

        void run();

        std::string getCommandList() const;

        class BravyBotException : public std::exception
        {
            private:
                const char *_msg;
            
            public:
                BravyBotException(const char *msg);
                virtual const char *what() const throw();
        };
};

#endif
