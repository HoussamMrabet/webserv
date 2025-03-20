/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 21:06:21 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/17 05:33:15 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <vector>

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
		
		std::string listen; // 8080 9999  array
		std::string serverNames;
		std::string root; // root folder
		std::string index; 
		std::string errorPages;
		std::string allowedMethods ; // 8080 9999  array
		std::string uploadDir; // upload directoryB     
		bool autoIndex;
		std::string host; //  127.0.0.1
		//rray of servernames a.com b.com
		// Errors : 404 404.html map
		// body size : size_t

		// array Locations
	public :
		Server(); 
		Server(const Server &copy);
		Server &operator = (const Server &copy);
		~Server();
		
};