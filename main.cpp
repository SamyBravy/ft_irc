#include "ft_irc.hpp"
#include "Server.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
	{
		std::cerr << "Error: usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}
	if (std::string(argv[1]).find_first_not_of("0123456789") != std::string::npos
        || strToNum<int>(argv[1]) < 1024 || strToNum<int>(argv[1]) > 49151)
	{
		std::cerr << "Error: port must be an integer between 1024 and 49151" << std::endl;
		return 1;
	}
    if (argv[2][0] == '\0')
    {
        std::cerr << "Error: password cannot be empty" << std::endl;
        return 1;
    }

	Server server(strToNum<int>(argv[1]), argv[2]);

	try
	{
		server.run();
        server.listenClients();
	}
	catch (const Server::ServerException &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}

std::vector<std::string> split(const std::string &str, char delim)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delim))
        tokens.push_back(token);

    return tokens;
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

std::string getDay(time_t t)
{
    struct tm *timeinfo = localtime(&t);
    char buffer[80];
    strftime(buffer, 80, "%a", timeinfo);
    return std::string(buffer);
}

std::string getDate(time_t t)
{
    struct tm *timeinfo = localtime(&t);
    char buffer[80];
    strftime(buffer, 80, "%d %b %Y", timeinfo);
    return std::string(buffer);
}

std::string getTime(time_t t)
{
    struct tm *timeinfo = localtime(&t);
    char buffer[80];
    strftime(buffer, 80, "%H:%M:%S", timeinfo);
    return std::string(buffer);
}
