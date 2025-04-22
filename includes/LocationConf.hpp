/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConf.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:43:14 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/22 07:08:07 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "ConfigBuilder.hpp"

class LocationConf {
    private :
		const std::string name;
        std::string root;                        
        std::vector<std::string> index;           
        bool autoIndex;
        std::vector<std::string> allowedMethods;
        size_t bodySizeLimit;
        std::string redirectUrl;
        bool    listing;
    public :
        LocationConf();
		LocationConf(const ServerConf server, std::string name);
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