/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConf.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:43:14 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/26 09:09:57 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "Config.hpp"

class Location {
    private :
        std::string root;                        
        std::vector<std::string> index;         
        std::vector<std::string> errorPages;  
        bool autoIndex;
        std::vector<std::string> allowedMethods;
        size_t bodySizeLimit;
    public :
        Location();
        Location(const Server &server);
        Location(unsigned int &start, std::vector<std::string> tokens);
        Location(const Location &copy);
        Location &operator=(const Location &copy);
        virtual ~Location();
        
};