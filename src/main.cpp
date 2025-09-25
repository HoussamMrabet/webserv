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
#include "Request.hpp"


std::vector<ServerConf> globalServer;

int main(int ac, char **av)
{
	// Request::users

	t_user user1;
    user1.username = "hmrabet";
    user1.password = "hmrabet123";
    user1.email = "hmrabet@student.1337.ma";
	user1.fullName = "Houssam Mrabet";
	user1.job = "Web Developer";
	user1.avatar = "./assets/houbet.jpeg";
    
    t_user user2;
    user2.username = "mel-hamd";
    user2.password = "mel-hamd123";
    user2.email = "mel-hamd@student.1337.ma";
	user2.fullName = "Mohammed El Hamdaoui";
	user2.job = "Backend Developer";
	user2.avatar = "./assets/mel-hamd.jpg";

    t_user user3;
    user3.username = "cmasnaou";
    user3.password = "cmasnaou123";
    user3.email = "cmasnaou@student.1337.ma";
	user3.fullName = "Chorouk Masnaoui";
	user3.job = "Frontend Developer";
	user3.avatar = "./assets/cmasnaou.jpg";

    // Assign them to the static vector
    Request::users.push_back(user1);
    Request::users.push_back(user2);
    Request::users.push_back(user3);

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