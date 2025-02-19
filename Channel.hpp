/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 22:15:27 by samuele           #+#    #+#             */
/*   Updated: 2025/02/18 22:16:52 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "ft_irc.hpp"

class Channel
{
	private:
		std::string _name;

	public:
		Channel(std::string name = "default");
		~Channel();
		
		void setName(const std::string &name) { _name = name; }
};

#endif
