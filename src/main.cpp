/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:12:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/04/21 06:49:51 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main()
{
    // try
    // {
    //     std::vector<std::string> tokens = TokenizeFile::tokens("config/default.conf");
    
    //     for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
    //         std::cout << *it << std::endl;
    //     }
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
	try {

    	std::vector<ServerConf> servers = ConfigBuilder::generateServers("config/default.conf");
		std::cout << servers.size() << std::endl;
		for (std::vector<ServerConf>::iterator it = servers.begin(); it != servers.end() ; it++) {
			std::cout << *it << std::endl;
		}
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
	}
    
    
    
    return (0);
}