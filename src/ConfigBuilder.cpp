/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 06:15:23 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 17:00:41 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigBuilder.hpp"

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
	// std::stack<char> stk;
	for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it ) {
		if (*it == "server" && (it + 1 ) != tokens.end() && *(it + 1) == "{" ) {
			std::cout << "there is a server !" << std::endl;
			it++;
			ConfigBuilder::buildServer(it, tokens);
			if (it == tokens.end())
				break;
		}
		else 
			{
				std::cout << "Error" << std::endl;
			}
		
	}
	return (res);
}

ServerConf ConfigBuilder::buildServer(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	ServerConf server;

	while (it != tokens.end())
	{
		if (*it == "listen") {
			it++;
			server.setListen(it, tokens);
			continue ;
		}
		// else if (*it == "server_name") {
		// 	server.setServerNames(it);
		// }
		// else if (*it == "root") {
		// 	server.setRoot(it);
		// }
		// else if (*it == "index") {
		// 	server.setIndex(it);
		// }
		// else if (*it == "error_page") {
		// 	server.setErrorPages(it);
		// }
		// else if (*it == "upload_directory") {
		// 	server.setUploadDir(it);
		// }
		// else if (*it == "auto_index") {
		// 	server.setAutoIndex(it);
		// }
		// else if (*it == "client_max_body_size") {
		// 	server.setBodySizeLimit(it);
		// }
		else if (*it == "location") {
			
		}
		else {
			
		}
		it++;
	}
	return (server);
}

bool ConfigBuilder::checkDirective(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens) {
	if (*it == "{" || *it == "}" || it == tokens.end())
		return (true);
	return (false);
}