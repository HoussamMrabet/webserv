/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:12:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/05/01 17:58:17 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <exception>
#include <signal.h>
#include "ServerBlock.hpp"
#include "WebServer.hpp"



int main(int ac, char **av)
{
	try{
        // signal(SIGPIPE, SIG_IGN); // sig ignore broken pipe,q to remove later!
		std::string config_file = "config/default.conf";
		if (ac > 1) config_file = av[1]; // parse config file name and path?
		std::vector<ServerConf> servers = ConfigBuilder::generateServers(config_file); // import servers from config file
		std::map<Listen, ServerBlock > serverBlocks = makeBloks(servers); // make blocks of servers sharing the same listen
        WebServ::startServer(serverBlocks);

	}
	catch (const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}

}