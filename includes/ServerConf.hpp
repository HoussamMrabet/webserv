
#pragma once 

#include "ConfigBuilder.hpp"

class LocationConf;


class ServerConf {

	private :
		std::vector<std::pair<std::string, std::string> > listen;    // getaddrinfo     
    	std::set<std::string> serverNames;    
    	std::string root;                   
    	std::vector<std::string> index;         
    	std::map<std::string, std::string> errorPages;   
    	std::string uploadDir;                   
    	bool autoIndex;                                                 
    	size_t bodySizeLimit;
        std::map<std::string, LocationConf> locations;
		bool ready;
	public :
		ServerConf();
		ServerConf(const ServerConf &copy);
		ServerConf &operator = (const ServerConf &copy);
		virtual ~ServerConf();
        
		void setListen(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens);
		void setServerNames(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setRoot(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setErrorPages(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setUploadDir(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setAutoIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setBodySizeLimit(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setLocations(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens, std::stack<char> &stk);
		
		std::vector<std::pair<std::string, std::string> > getListen() const;
		std::set<std::string> getServerNames() const;
		std::string getRoot() const;
		std::vector<std::string> getIndex() const;
		std::map<std::string, std::string> getErrorPages() const;
		std::string getUploadDir() const;
		bool getAutoIndex() const;
		size_t getBodySizeLimit() const;
		std::map<std::string, LocationConf> getLocations() const;
		bool getReady() const;

		void printListen(std::ostream& os) const;
		void printServerNames(std::ostream& os) const;
		void printRoot(std::ostream& os) const;
		void printIndex(std::ostream& os) const;
		void printErrorPages(std::ostream& os) const;
		void printUploadDir(std::ostream& os) const;
		void printAutoIndex(std::ostream& os) const;
		void printBodySizeLimit(std::ostream& os) const;
		void printLocations(std::ostream& os) const;
		void printReady(std::ostream& os) const;

		static std::pair<std::string, std::string> parseListen(std::string str);

		class ParseError : public std::exception {
			private :
				std::string message;
			public :
				ParseError(const std::string str) {
					this->message = str;
				}
				const char* what() const throw() {
					return (this->message.c_str());
				}
				~ParseError() throw() {
					
				}

		};

		class InvalidValue : public std::exception {
			private :
				std::string message;
			public :
				InvalidValue(const std::string str) {
					this->message = str;
				}
				const char* what() const throw() {
					return (this->message.c_str());
				}
				~InvalidValue() throw() {

				}

		};
};

std::ostream& operator << (std::ostream& os, const ServerConf server);