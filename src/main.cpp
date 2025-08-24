/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:12:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/22 03:17:21 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <exception>
#include <signal.h>
// #include "ServerConf.hpp"
#include "WebServer.hpp"

std::vector<ServerConf> globalServer;

int main(int ac, char **av)
{
	try{
        // signal(SIGPIPE, SIG_IGN); // sig ignore broken pipe,q to remove later!
		std::string config_file = "config/default.conf";
		if (ac > 1) config_file = av[1]; // parse config file name and path?
		globalServer = ConfigBuilder::generateServers(config_file); // import servers from config file
		std::vector<ServerConf> servers = globalServer; // import servers from config file
		ServerConf server = servers[0]; // vector should have only one vector
		std::cout << "Server data\n";
		std::cout << server << std::endl;
		WebServ::startServer(server);

	}
	catch (const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}

}