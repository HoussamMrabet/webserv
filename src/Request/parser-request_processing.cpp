/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-request_processing.cpp                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 20:04:44 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/24 19:57:00 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "WebServer.hpp"

// std::string methodToString(t_method method)
// {
//     switch (method)
//     {
//     case GET:
//         return "GET";
//     case POST:
//         return "POST";
//     case DELETE:
//         return "DELETE";
//     default:
//         return "UNDEFINED";
//     }
// }

void Request::processResponseErrors()
{
    std::string methodStr = getStrMethod();
    // std::string methodStr = methodToString(this->method);

    const ServerConf &server = globalServer[0];

    std::map<std::string, LocationConf> locations = server.getLocations();
    std::map<std::string, LocationConf>::iterator locIt = locations.find(this->location);

    if (locIt == locations.end())
    {
        this->message = "Route Not Found";
        throw 404;
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
        throw 405;
    }

    return;
}