/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConf.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 08:43:14 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/22 11:47:04 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "ConfigBuilder.hpp"

class ServerConf;

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
        
		void setRoot(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setAutoIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setAllowedMethods(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
        void setBodySizeLimit(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
        void setRedirectUrl(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
        void setListing(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		
		const std::string getName() const;
        std::string getRoot() const;                        
        std::vector<std::string> getIndex() const;           
        bool getAutoIndex() const;
        std::vector<std::string> getAllowedMethods() const;
        size_t getBodySizeLimit() const;
        std::string getRedirectUrl() const;
        bool    getListing() const;
		
    };