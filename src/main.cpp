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

#include "ConfigBuilder.hpp"
#include "ServerConf.hpp"
#include "WebServ.hpp"
#include "Request.hpp"
#include <iostream>
#include <exception>
#include <csignal>

void signalHandler(int)
{
	WebServ::_runServer = false;
}

std::string getTime()
{
    time_t now = time(0);
    struct tm* time = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", time);
	return (buffer);
}

int main(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << "Usage: ./webserv [config_file]" << std::endl;
		return (1);
    }
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

    Request::users.push_back(user1);
    Request::users.push_back(user2);
    Request::users.push_back(user3);

	signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);

	try{
		std::string config_file = ac > 1? av[1]: "config/default.conf";
		std::vector<ServerConf> globalServer = ConfigBuilder::generateServers(config_file);
		std::cout << "Webserv 1.0 Development Server started at " << getTime() << std::endl;
		ServerConf server = ConfigBuilder::getServer(); // one server
		WebServ::startServer(server);
	}
	catch (const std::exception& ex){
		std::cerr << ex.what() << std::endl;
		return (1);
	}
	return (0);

}