/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 10:48:31 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 13:35:36 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConf.hpp"



ServerConf::ServerConf() {
	
};
ServerConf::ServerConf(const ServerConf &copy) {
	
}
ServerConf &ServerConf::operator = (const ServerConf &copy) {
	if (this->listen != copy.getListen()) {
		this->autoIndex = copy.getAutoIndex();
		this->bodySizeLimit = copy.getBodySizeLimit();
		this->errorPages = copy.getErrorPages();
		this->index = copy.getAutoIndex();
		this->listen = copy.getListen();
		this->locations = copy.getLocations();
		this->root = copy.getRoot();
		this->serverNames = copy.GetServerNames();
	}
	return (*this);
};

ServerConf::~ServerConf() {
	
}

std::vector<std::pair<std::string, std::string> > ServerConf::getListen() {
	return (this->listen);
}
std::vector<std::string> ServerConf::getServerNames() {
	return (this->serverNames);
}
std::string getRoot() {
	return (this->root);
}
std::vector<std::string> ServerConf::getIndex() {
	return (this->index);
}
std::map<int, std::string> ServerConf::getErrorPages() {
	return (this->errorPages);
}
std::string ServerConf::getUploadDir() {
	return (this->uploadDir);
}
bool ServerConf::getAutoIndex() {
	return (this->autoIndex);
}
size_t ServerConf::getBodySizeLimit() {
	return (this->bodySizeLimit);
}
std::map<std::string, LocationConf> ServerConf::getLocations() {
	return (this->locations);
}