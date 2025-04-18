/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:41:18 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 13:20:36 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "ConfigBuilder.hpp"


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
        
		void setListen(std::vector<std::string>::const_iterator &it);
		void setServerNames(std::vector<std::string>::const_iterator &it);
		void setRoot(std::vector<std::string>::const_iterator &it);
		void setIndex(std::vector<std::string>::const_iterator &it);
		void setErrorPages(std::vector<std::string>::const_iterator &it);
		void setUploadDir(std::vector<std::string>::const_iterator &it);
		void setAutoIndex(std::vector<std::string>::const_iterator &it);
		void setBodySizeLimit(std::vector<std::string>::const_iterator &it);
		void setLocations(std::map<std::string, LocationConf> locations);
		
		std::vector<std::pair<std::string, std::string> > getListen();
		std::vector<std::string> getServerNames();
		std::string getRoot();
		std::vector<std::string> getIndex();
		std::map<int, std::string> getErrorPages();
		std::string getUploadDir();
		bool getAutoIndex();
		size_t getBodySizeLimit();
		std::map<std::string, LocationConf> getLocations();
};

