/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 10:48:31 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/20 06:06:14 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ServerConf.hpp"



ServerConf::ServerConf() {
	
};
ServerConf::ServerConf(const ServerConf &copy) {
	*this = copy;
}
ServerConf &ServerConf::operator = (const ServerConf &copy) {
	if (this->listen != copy.getListen()) {
		this->autoIndex = copy.getAutoIndex();
		this->bodySizeLimit = copy.getBodySizeLimit();
		this->errorPages = copy.getErrorPages();
		this->index = copy.getIndex();
		this->listen = copy.getListen();
		this->locations = copy.getLocations();
		this->root = copy.getRoot();
		this->serverNames = copy.getServerNames();
	}
	return (*this);
};

ServerConf::~ServerConf() {
	
}

std::vector<std::pair<std::string, std::string> > ServerConf::getListen() const {
	return (this->listen);
}
std::vector<std::string> ServerConf::getServerNames() const {
	return (this->serverNames);
}
std::string ServerConf::getRoot() const {
	return (this->root);
}
std::vector<std::string> ServerConf::getIndex() const {
	return (this->index);
}
std::map<int, std::string> ServerConf::getErrorPages() const {
	return (this->errorPages);
}
std::string ServerConf::getUploadDir() const {
	return (this->uploadDir);
}
bool ServerConf::getAutoIndex() const {
	return (this->autoIndex);
}
size_t ServerConf::getBodySizeLimit() const {
	return (this->bodySizeLimit);
}
std::map<std::string, LocationConf> ServerConf::getLocations() const {
	return (this->locations);
}

void ServerConf::setListen(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		while (*it != ";")
		{
			if (ConfigBuilder::checkDirective(it,   tokens))
				throw ServerConf::ParseError("Config file : syntax error !");
			std::cout << *it << std::endl;
			it++;
		}
	}
}


std::pair<std::string, std::string> ServerConf::parseListen(std::string str) {
	
	std::pair<std::string, std::string>  res;
	size_t pos;
	if ((pos = str.find(":")) != std::string::npos) {
		std::string  val1 = str.substr(0, pos);
		std::string val2 = str.substr(pos + 1);
	}
	else {
		
	}
}