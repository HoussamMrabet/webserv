/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConf.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 16:33:00 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/22 12:56:48 by mel-hamd         ###   ########.fr       */
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