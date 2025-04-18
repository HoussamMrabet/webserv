/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 06:15:23 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 07:23:52 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/config/ConfigBuilder.hpp"

ConfigBuilder::ConfigBuilder() {
	
}

ConfigBuilder::ConfigBuilder(const ConfigBuilder &copy) {

		*this = copy;

}

ConfigBuilder &ConfigBuilder::operator = (const ConfigBuilder &copy) {

		if (this != &copy)
		{
			return (*this);
		}
		return (*this);
}

ConfigBuilder::~ConfigBuilder() {
	
}

std::vector<ServerConf> ConfigBuilder::generateServers(std::string file) {
	std::vector<ServerConf>  res;
	std::vector<std::string> tokens = TokenizeFile::tokens(file);
	for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it ) {
		if (*it == "server" && *(it + 1) == "{") {
			std::cout << "there is a server !" << std::endl;	
		}
	}
	return (res);
}