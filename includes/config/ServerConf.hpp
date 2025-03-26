/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:41:18 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/26 09:21:49 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "Config.hpp"


class Server {

	private :
		std::vector<std::pair<std::string, int>> listen;         
    	std::vector<std::string> serverNames;    
    	std::string root;                  
    	std::vector<std::string> index;         
    	std::map<int, std::string> errorPages;   
    	std::string uploadDir;                   
    	bool autoIndex;                          
    	std::string host;                       
    	size_t bodySizeLimit;
        std::map<std::string, Location> locations;
	public :
		Server();
        Server(unsigned int &start, std::vector<std::string> tokens); 
		Server(const Server &copy);
		Server &operator = (const Server &copy);
		virtual ~Server();
		
};

