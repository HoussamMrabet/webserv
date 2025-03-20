/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: voop <voop@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 21:06:21 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/20 10:23:06 by voop             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <vector>
# include <map>

/*

location /images/ {
        root /data;
    }

class Location {

	private :
		
		std::string index; 
		bool autoIndex; 
		std::string location ; \\  /images/
		std::string root; // root folder
		// Errors : 404 404.html map
		// return pair [sraruseCode, to]	
		std::string allowedMethods ; // 8080 9999  array
		
	public :
	
	
	
};
*/

class Server {

	private :
		std::vector<std::string> listen;         
    	std::vector<std::string> serverNames;    
    	std::string root;                        
    	std::vector<std::string> index;         
    	std::map<int, std::string> errorPages;   
    	std::vector<std::string> allowedMethods; 
    	std::string uploadDir;                   
    	bool autoIndex;                          
    	std::string host;                       
    	size_t bodySizeLimit;
	public :
		Server(); 
		Server(const Server &copy);
		Server &operator = (const Server &copy);
		~Server();
		
};