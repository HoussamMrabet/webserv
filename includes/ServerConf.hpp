/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:41:18 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/20 05:54:12 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "ConfigBuilder.hpp"

class LocationConf;


class ServerConf {

	private :
		std::vector<std::pair<std::string, std::string> > listen;    // getaddrinfo     
    	std::vector<std::string> serverNames;    
    	std::string root;                  
    	std::vector<std::string> index;         
    	std::map<int, std::string> errorPages;   
    	std::string uploadDir;                   
    	bool autoIndex;                                                 
    	size_t bodySizeLimit;
        std::map<std::string, LocationConf> locations; 
	public :
		ServerConf();
		ServerConf(const ServerConf &copy);
		ServerConf &operator = (const ServerConf &copy);
		virtual ~ServerConf();
        
		void setListen(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens);
		void setServerNames(std::vector<std::string>::const_iterator &it);
		void setRoot(std::vector<std::string>::const_iterator &it);
		void setIndex(std::vector<std::string>::const_iterator &it);
		void setErrorPages(std::vector<std::string>::const_iterator &it);
		void setUploadDir(std::vector<std::string>::const_iterator &it);
		void setAutoIndex(std::vector<std::string>::const_iterator &it);
		void setBodySizeLimit(std::vector<std::string>::const_iterator &it);
		void setLocations(std::map<std::string, LocationConf> locations);
		
		std::vector<std::pair<std::string, std::string> > getListen() const;
		std::vector<std::string> getServerNames() const;
		std::string getRoot() const;
		std::vector<std::string> getIndex() const;
		std::map<int, std::string> getErrorPages() const;
		std::string getUploadDir() const;
		bool getAutoIndex() const;
		size_t getBodySizeLimit() const;
		std::map<std::string, LocationConf> getLocations() const;

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

