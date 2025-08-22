/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConf.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 16:33:00 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/08/22 01:58:49 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConf.hpp"


LocationConf::LocationConf() {

}

LocationConf::LocationConf(const ServerConf server, std::string name) : name(name) {
	this->root = server.getRoot();
	this->index = server.getIndex();
	this->autoIndex = server.getAutoIndex();
	this->bodySizeLimit = server.getBodySizeLimit();
	allowedMethods.push_back("GET");
	allowedMethods.push_back("POST");
	allowedMethods.push_back("DELETE");
	this->listing = false;
}

LocationConf::~LocationConf() {

}


void LocationConf::setRoot(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		this->root = *it;
		it++;
		if (*it != ";")
			throw ServerConf::ParseError("Config file : Root directive cant have multiple values !") ;
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


void LocationConf::setIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it, tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Confi file : empty value not accepted !");
		while (*it != ";")
		{
			if (ConfigBuilder::checkDirective(it, tokens))
				throw ServerConf::ParseError("Config file : syntax error !");
			this->index.push_back(*it);
			it++;
		}
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


void LocationConf::setAutoIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it, tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		if (*it != "off" && *it != "on")
			throw ServerConf::ParseError("Config file : auto_index must be on or off!");
		if (*it == "on")
			this->autoIndex = true;
		else
			this->autoIndex = false;
		it++;
		if (*it != ";")
			throw ServerConf::ParseError("Config file : auto_index directive cant have multiple values !") ;
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


void LocationConf::setAllowedMethods(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Confi file : empty value not accepted !");
		std::vector<std::string> res;
		while (*it != ";")
		{
			if (ConfigBuilder::checkDirective(it,   tokens))
				throw ServerConf::ParseError("Config file : syntax error !");
			if (*it != "GET" && *it != "POST" && *it != "DELETE")
				throw ServerConf::ParseError("Config file : allowed methods accept only POST , GET , DELETE methods !");
			for (std::vector<std::string>::iterator its = res.begin(); its != res.end(); its++) {
				if (*its == *it)
					throw ServerConf::ParseError("Config file : allowed methods cant have douuplicated values !");
			}
			res.push_back(*it);
			it++;
		}
		this->allowedMethods = res;
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


void LocationConf::setBodySizeLimit(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		char *stops = NULL;
		unsigned long res;
		errno = 0;
		res = std::strtoul((*it).c_str(), &stops, 10);
		if (stops &&  *stops !='\0')
			throw ServerConf::ParseError("Config file : cant convert the client_max_body_size !");
		if (errno == ERANGE)
			throw ServerConf::ParseError("Config file : client_max_body_size overflow !");
		if ((*it)[0] == '-')
				throw ServerConf::ParseError("Config file : cant convert the client_max_body_size cause its Negative !");
		this->bodySizeLimit = res;
		it++;
		if (*it != ";")
			throw ServerConf::ParseError("Config file : client_max_body_size directive cant have multiple values !") ;
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


void LocationConf::setListing(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		if (*it != "off" && *it != "on")
			throw ServerConf::ParseError("Config file : listing must be on or off!");
		if (*it == "on")
			this->listing = true;
		else
			this->listing = false;
		it++;
		if (*it != ";")
			throw ServerConf::ParseError("Config file : listing directive cant have multiple values !") ;
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


void LocationConf::setRedirectUrl(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
		if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		if (*it != "301")
			throw ServerConf::ParseError("Config file : unvalid return status code !");
		it++;
		if (ConfigBuilder::checkDirective(it,   tokens))
			throw ServerConf::ParseError("Config file : Syntax error !");
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		this->redirectUrl = *it;
		it++;
		if (*it != ";")
			throw ServerConf::ParseError("Config file : return directive cant have multiple values !") ;
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


const std::string LocationConf::getName() const {
	return (this->name);
}

std::string LocationConf::getRoot() const {
	return (this->root);
}                        

std::vector<std::string> LocationConf::getIndex() const {
	return (this->index);
}         

bool LocationConf::getAutoIndex() const {
	return (this->autoIndex);
}

std::vector<std::string> LocationConf::getAllowedMethods() const {
	return (this->allowedMethods);
}

size_t LocationConf::getBodySizeLimit() const {
	return (this->bodySizeLimit);
}

std::string LocationConf::getRedirectUrl() const {
	return (this->redirectUrl);
}

bool    LocationConf::getListing() const {
	return (this->listing);
}


void LocationConf::printName(std::ostream& os) const {
	os << "location : " << this->getName() << std::endl;
}
void LocationConf::printRoot(std::ostream& os) const {
	os << "|->root : " << this->getRoot() << std::endl;
}
void LocationConf::printIndex(std::ostream& os) const {
	os << "|-> index : " ;
	for (std::vector<std::string>::const_iterator it = this->index.begin(); it != this->index.end() ; it++) {
		os << *it << " | ";
	}
	os << std::endl;
}
void LocationConf::printAutoIndex(std::ostream& os) const {
	os << "|-> index : " ;
	if (this->autoIndex)
		os << "on" << std::endl;
	else
		os << "off" << std::endl;
}
void LocationConf::printAllowedMethods(std::ostream& os) const {
	os << "|-> allowed methods : " ;
	for (std::vector<std::string>::const_iterator it = this->allowedMethods.begin(); it != this->allowedMethods.end() ; it++) {
		os << *it << " | ";
	}
	os << std::endl;
}
void LocationConf::printBodySizeLimit(std::ostream& os) const {
	os << "|-> body size limit : "  << this->bodySizeLimit << std::endl;
}
void LocationConf::printRedirectUrl(std::ostream& os) const {
	os << "|-> redirect URL : "  << this->redirectUrl << std::endl;
}
void LocationConf::prontListing(std::ostream& os) const {
	os << "|-> listing : " ;
	if (this->listing)
		os << "on" << std::endl;
	else
		os << "off" << std::endl;
}