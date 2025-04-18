/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:41:18 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 07:11:24 by mel-hamd         ###   ########.fr       */
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
        ServerConf(unsigned int &start, std::vector<std::string> tokens); 
		ServerConf(const ServerConf &copy);
		ServerConf &operator = (const ServerConf &copy);
		virtual ~ServerConf();
        
		void setListen(unsigned int &start, std::vector<std::string> tokens);
		void setServerNames(unsigned int &start, std::vector<std::string> tokens);
		void setRoot(unsigned int &start, std::vector<std::string> tokens);
		void setIndex(unsigned int &start, std::vector<std::string> tokens);
		void setErrorPages(unsigned int &start, std::vector<std::string> tokens);
		void setUploadDir(unsigned int &start, std::vector<std::string> tokens);
		void setAutoIndex(unsigned int &start, std::vector<std::string> tokens);
		void setBodySizeLimit(unsigned int &start, std::vector<std::string> tokens);
		void setLocations(std::map<std::string, LocationConf> locations);
		
};

