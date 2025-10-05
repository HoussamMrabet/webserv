/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 06:15:23 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/08/21 19:40:30 by hmrabet          ###   ########.fr       */
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
	ServerConf temp;
	std::vector<std::string> tokens = TokenizeFile::tokens(file);
	std::stack<char> stk;
	for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it ) {
		if (*it == "server" && (it + 1 ) != tokens.end() && *(it + 1) == "{" ) {
			// std::cout << "there is a server !" << std::endl;
			it++;
			temp = ConfigBuilder::buildServer(it, tokens, stk);
			res.push_back(temp);
			if (it == tokens.end())
				break;
		}
		else 
		{
				throw ConfigBuilder::ErrorConfig("Config file : Error near to : " + std::string(*it));
		}
	}
	if (!stk.empty())
		throw ConfigBuilder::ErrorConfig("Config file : You have to open and close curly bracets { }");
	if (res.size() != 1)
		throw ConfigBuilder::ErrorConfig("Config file : should have only one server");


	if (res[0].getListen().empty()) {
		throw ConfigBuilder::ErrorConfig("Config file : 'listen' directive is required and cannot be empty");
	}

	std::cout << "Server configuration loaded successfully.\n";
	res[0].printListen(std::cout);
	return (res);
}

ServerConf ConfigBuilder::buildServer(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens, std::stack<char> &stk) {
	ServerConf server;

	while (it != tokens.end())
	{
		if (*it == "listen") {
			it++;
			server.setListen(it, tokens);
			continue; 
		}
		else if (*it == "server_name") {
			it++;
			server.setServerNames(it, tokens);
			continue;
		}
		else if (*it == "root") {
			it++;
			server.setRoot(it, tokens);
			continue;
		}
		else if (*it == "index") {
			it++;
			server.setIndex(it, tokens);
			continue ;
		}
		else if (*it == "error_page") {
			it++;
			server.setErrorPages(it, tokens);
			continue;
		}
		else if (*it == "upload_directory") {
			it++;
			server.setUploadDir(it, tokens);
			continue;
		}
		else if (*it == "auto_index") {
			it++;
			server.setAutoIndex(it, tokens);
			continue;
		}
		else if (*it == "client_max_body_size") {
			it++;
			server.setBodySizeLimit(it, tokens);
			continue;
		}
		else if (*it == "location") {
			it++;
			server.setLocations(it, tokens, stk);
			continue;
		}
		else if (*it == "{") {
			stk.push('{');
		}
		else if (*it == "}") {
			stk.pop();
		}
		else if (*it == ";") {
			throw ConfigBuilder::ErrorConfig("Config file : error et \";\"");
		}
		else {
			throw ConfigBuilder::ErrorConfig("Config file : the directive "+(*it)+" is not supported !");
		}
		if (stk.size() == 0)
			break;
		it++;
	}
	// if (server.getReady() == false)
	// 	throw ConfigBuilder::ErrorConfig("Error : your server should have directives to be able to run !");
	// std::cout << server.getUploadDir() << std::endl << server << std::endl;
	// std::cout << "_____________________________" <<std::endl;
	// server.printLocations(std::cout);
	return (server);
}

bool ConfigBuilder::checkDirective(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens) {
	if (it == tokens.end()  || *it == "{" || *it == "}")
		return (true);
	return (false);
}


bool ConfigBuilder::checkPort(std::string str) {
	if (str.empty())
		return (true);
	for (size_t i = 0 ; i < str.length() ; i++) {
		if (str[i]  < 48 || str[i] > 57)
			return (true);
	}
	if (str.length() > 5)
		return (true);
	int num = std::atoi(str.c_str());
	if (num < 1 || num > 65535)
		return (true);
	return (false);
}

bool ConfigBuilder::checkIp(std::string& str) {
	
	std::stringstream ss(str);
	int count = 0;
	std::string buff;
	int test;

	if (str == "localhost"){
		str = "127.0.0.1";
		return (false);
	}
	while (std::getline(ss, buff, '.')) {
		count++;
		if (count > 4)
			return (true);
		if (buff.length() > 3)
			return (true);
		for (size_t i = 0; i < buff.length() ; i++) {
			if (buff[i]  < 48 || buff[i] > 57)
				return (true);
		}
		test = std::atoi(buff.c_str());
		if (test < 0 || test > 255 )
			return (true);
	}
	if (count != 4)
		return (true);
	return (false);
}
