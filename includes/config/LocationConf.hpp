/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConf.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:43:14 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 07:10:38 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "ConfigBuilder.hpp"

class LocationConf {
    private :
        std::string root;                        
        std::vector<std::string> index;         
        std::map<int,std::string> errorPages;  
        bool autoIndex;
        std::vector<std::string> allowedMethods;
        size_t bodySizeLimit;
        std::string redirectUrl;
        bool    listing;
    public :
        LocationConf();
        LocationConf(unsigned int &start, std::vector<std::string> tokens);
        LocationConf(const LocationConf &copy);
        LocationConf &operator=(const LocationConf &copy);
        virtual ~LocationConf();
        
		void setRoot(unsigned int &start, std::vector<std::string> tokens);
		void setIndex(unsigned int &start, std::vector<std::string> tokens);
		void setErrorPages(unsigned int &start, std::vector<std::string> tokens);
		void setAutoIndex(unsigned int &start, std::vector<std::string> tokens);
		void setAllowedMethods(unsigned int &start, std::vector<std::string> tokens);
        void setBodySizeLimit(unsigned int &start, std::vector<std::string> tokens);
        void setRedirectUrl(unsigned int &start, std::vector<std::string> tokens);
        void setListing(unsigned int &start, std::vector<std::string> tokens);
    };