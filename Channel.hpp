#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "ft_irc.hpp"

class Channel
{
	private:
		std::string _name;

	public:
		Channel(const std::string &name = "default");
		~Channel();
		
		void setName(const std::string &name) { _name = name; }
};

#endif
