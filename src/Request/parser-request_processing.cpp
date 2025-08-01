/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-request_processing.cpp                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 20:04:44 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/01 10:25:04 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "ConfigBuilder.hpp"

std::string methodToString(t_method method)
{
    switch (method)
    {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case DELETE:
            return "DELETE";
        default:
            return "UNDEFINED";
    }
}

void Request::processResponseErrors()
{
    std::vector<ServerConf> servers = ConfigBuilder::generateServers("./config.conf");
    std::string methodStr = methodToString(this->method);

    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConf &server = servers[i];

        std::set<std::string> serverNames = server.getServerNames();
        if (serverNames.find(this->host) != serverNames.end())
        {

            std::map<std::string, LocationConf> locations = server.getLocations();
            std::map<std::string, LocationConf>::iterator locIt = locations.find(this->uri);

            if (locIt == locations.end())
            {
                this->message = "Method Not Allowed";
                throw 403;
            }

            LocationConf location = locIt->second;

            this->bodySizeLimit = location.getBodySizeLimit();
            this->root = location.getRoot();

            std::vector<std::string> allowedMethods = location.getAllowedMethods();

            bool methodAllowed = false;
            for (size_t j = 0; j < allowedMethods.size(); ++j)
            {
                if (allowedMethods[j] == methodStr)
                {
                    methodAllowed = true;
                    break;
                }
            }

            if (!methodAllowed)
            {
                this->message = "Method Not Allowed";
                throw 403;
            }

            return;
        }
    }
    
    throw 403;
}