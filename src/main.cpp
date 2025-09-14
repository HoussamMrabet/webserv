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
#include <signal.h>


std::vector<ServerConf> globalServer;

int main(int ac, char **av)
{
	// signal(SIGPIPE, SIG_IGN);  // Ignore the signal leads to infinit loop!!!
	std::cout << "Webserv 14.9.25 Development Server started at Sun Sep 14 15:22:14 2025\n";
	try{
        // signal(SIGPIPE, SIG_IGN); // sig ignore broken pipe,q to remove later!
		std::string config_file = "config/default.conf";
		if (ac > 1) config_file = av[1]; // parse config file name and path? 
		globalServer = ConfigBuilder::generateServers(config_file); // import servers from config file
		std::vector<ServerConf> servers = globalServer; // import servers from config file
		ServerConf server = servers[0]; // vector should have only one vector
		MOHAMED && std::cout << "Server data\n";
		MOHAMED && std::cout << server << std::endl;
		WebServ::startServer(server);

	}
	catch (const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}

}