/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 10:48:31 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/22 15:00:15 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ServerConf.hpp"



ServerConf::ServerConf() {
	this->ready = false;
	this->bodySizeLimit = 1024;
	this->autoIndex = false;
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
		this->uploadDir = copy.getUploadDir();
	}
	return (*this);
};

ServerConf::~ServerConf() {
	
}

std::vector<std::pair<std::string, std::string> > ServerConf::getListen() const {
	return (this->listen);
}
std::set<std::string> ServerConf::getServerNames() const {
	return (this->serverNames);
}
std::string ServerConf::getRoot() const {
	return (this->root);
}
std::vector<std::string> ServerConf::getIndex() const {
	return (this->index);
}
std::map<std::string, std::string> ServerConf::getErrorPages() const {
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

bool ServerConf::getReady() const {
	return (this->ready);
}

void ServerConf::printListen(std::ostream& os) const {
	for (size_t i = 0; i < this->listen.size(); i++) {
		os << "--> " << listen[i].first << ":" << listen[i].second << std::endl;
	}
}
void  ServerConf::printServerNames(std::ostream& os) const {
	for (std::set<std::string>::const_iterator it = this->serverNames.begin() ; it !=  serverNames.end() ; it++) {
		os << "--> " << *it << std::endl;
	}
}
void  ServerConf::printRoot(std::ostream& os) const {
	os << "-->" << this->root << std::endl;
}
void  ServerConf::printIndex(std::ostream& os) const {
	for (std::vector<std::string>::const_iterator it = this->index.begin(); it != this->index.end() ; it++) {
		os << "--> " << *it << std::endl;
	}
}
void  ServerConf::printErrorPages(std::ostream& os) const {
	for (std::map<std::string, std::string>::const_iterator it = this->errorPages.begin(); it != this->errorPages.end(); it++) {
		os << "--> " << it->first << " " << it->second << std::endl;
	}
}
void ServerConf::printUploadDir(std::ostream& os) const {
		os << "-->" << this->uploadDir << std::endl;
}
void  ServerConf::printAutoIndex(std::ostream& os) const {
	if (this->autoIndex)
		os << "--> " << "auto index : on"<< std::endl;
	else
		os << "--> " << "auto index : off"<< std::endl;
}
void ServerConf::printBodySizeLimit(std::ostream& os) const {
	os << "--> " << this->bodySizeLimit << std::endl;
}
void ServerConf::printLocations(std::ostream& os) const {
	for (std::map<std::string, LocationConf>::const_iterator it = this->locations.begin();  it != this->locations.end() ; it++)
	{
		os << "----------------------------" << std::endl;
		it->second.printName(os);
		it->second.printRoot(os);
		it->second.printIndex(os);
		it->second.printAutoIndex(os);
		it->second.printAllowedMethods(os);
		it->second.printBodySizeLimit(os);
		it->second.printRedirectUrl(os);
		it->second.prontListing(os);
		os << "----------------------------" << std::endl;
	}
}

void ServerConf::printReady(std::ostream& os) const {
	os << "" << std::endl;
}


void ServerConf::setListen(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Confi file : empty value not accepted !");
		while (*it != ";")
		{
			if (ConfigBuilder::checkDirective(it,   tokens))
				throw ServerConf::ParseError("Config file : syntax error !");
			this->listen.push_back(ServerConf::parseListen(*it));
			it++;
		}
		it++;
	}
	else
		throw ServerConf::ParseError("Confi file : Syntax error !");
}


void ServerConf::setServerNames(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Confi file : empty value not accepted !");
		while (*it != ";")
		{
			if (ConfigBuilder::checkDirective(it,   tokens))
				throw ServerConf::ParseError("Config file : syntax error !");
			this->serverNames.insert(*it);
			it++;
		}
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}

void ServerConf::setRoot(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
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

void ServerConf::setIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Confi file : empty value not accepted !");
		while (*it != ";")
		{
			if (ConfigBuilder::checkDirective(it,   tokens))
				throw ServerConf::ParseError("Config file : syntax error !");
			this->index.push_back(*it);
			it++;
		}
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}

void ServerConf::setUploadDir(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		this->uploadDir = *it;
		it++;
		if (*it != ";")
			throw ServerConf::ParseError("Config file : Upload_directory directive cant have multiple values !") ;
		it++;
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}

void ServerConf::setAutoIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
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


void ServerConf::setErrorPages(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens) {
	int  test;
	std::string tmp;
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		if ((*it).length() != 3)
			throw ServerConf::ParseError("Config file : value : "+(*it)+" not accepted in  error_page !");
		test = std::atoi((*it).c_str());
		if (test < 400 || test >= 600)
			throw ServerConf::ParseError("Config file : value : "+(*it)+" not accepted in  error_page status code 4** or 5** !"); 
		
		tmp = *it;
		it++;
		if (ConfigBuilder::checkDirective(it,   tokens))
			throw ServerConf::ParseError("Config file : Root directive cant have multiple values !") ;
		if (*it == ";")
			throw ServerConf::ParseError("Config file : empty second value not accepted !");
		this->errorPages.insert(std::make_pair(tmp, *it));
		it++;
		if (*it != ";")
			throw ServerConf::ParseError("Config file : Root directive cant have multiple values !") ;
		it++;
	}
	else
		throw ServerConf::ParseError("Confi file : Syntax error !");
}

void ServerConf::setBodySizeLimit(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens) {
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

void ServerConf::setLocations(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens, std::stack<char> &stk) {
	if (!ConfigBuilder::checkDirective(it,   tokens))
	{
		if (*it == ";") 
			throw ServerConf::ParseError("Config file : empty value not accepted !");
		LocationConf location(*this, *it);
		it++;
		if (!ConfigBuilder::checkDirective(it,   tokens))
			throw ServerConf::ParseError("Config file : Syntax Error !");
		while(it != tokens.end()) {
			if (*it == "root") {
				it++;
				location.setRoot(it, tokens);
				continue ; 
			}
			else if (*it == "auto_index")
			{
				it++;
				location.setAutoIndex(it, tokens);
				continue ; 
			}
			else if (*it == "index")
			{
				it++;
				location.setIndex(it, tokens);
				continue ; 
			}
			else if (*it == "allowed_methods")
			{
				it++;
				location.setAllowedMethods(it, tokens);
				continue;
			}
			else if (*it == "client_max_body_size") {
				it++;
				location.setBodySizeLimit(it, tokens);
				continue;
			}
			else if (*it == "listing") {
				it++;
				location.setListing(it, tokens);
				continue;
			}
			else if (*it == "return") {
				it++;
				location.setRedirectUrl(it, tokens);
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
			it++;
			if (stk.size() == 1)
				break;
		}
		this->locations.insert(std::make_pair(location.getName(), location));
	}
	else
		throw ServerConf::ParseError("Config file : Syntax error !");
}


std::pair<std::string, std::string> ServerConf::parseListen(std::string str) {
	
	std::pair<std::string, std::string>  res;
	size_t pos;
	if ((pos = str.find(":")) != std::string::npos) {
		std::string  val1 = str.substr(0, pos);
		std::string val2 = str.substr(pos + 1);
		if (ConfigBuilder::checkPort(val2))
			throw ServerConf::InvalidValue("Config file : Not valid port at " + str );
		if (ConfigBuilder::checkIp(val1))
			throw ServerConf::InvalidValue("Config file : Not valid Ip address at " + str );
		res = make_pair(val1, val2);
	}
	else {
		if (!ConfigBuilder::checkPort(str))
			res = make_pair("127.0.0.1", str);
		else if (!ConfigBuilder::checkIp(str))
			res = make_pair(str, "8080");
		else
			throw ServerConf::InvalidValue("Config file : Not valid value at " + str);
	}
	return (res);
}


std::ostream& operator << (std::ostream& os, const ServerConf server) {
		os << "Server Class : " << std::endl;
		os << "#" << "Listen : " << std::endl;
		server.printListen(os);
		os << "#" << "Server Names : " << std::endl;
		server.printServerNames(os);
		os << "#" << "Root : " << std::endl;
		server.printRoot(os);
		os << "#" << "Index : " << std::endl;
		server.printIndex(os);
		os << "#" << "Error pages : " << std::endl;
		server.printErrorPages(os);
		os << "#" << "Upload directory : " << std::endl;
		server.printUploadDir(os);
		os << "#" << " Auto Index " << std::endl;
		server.printAutoIndex(os);
		os << "#" << "Body size limite : " << std::endl;
		server.printBodySizeLimit(os);
		os << "#" << "Locations : " << std::endl;
		server.printLocations(os);
	return (os);
}