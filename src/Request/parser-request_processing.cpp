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
#include "WebServ.hpp"

void Request::processResponseErrors()
{
    std::string methodStr = getStrMethod();
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

void Request::processRequest()
{
    std::map<std::string, LocationConf> locations = server.getLocations();
    std::string document_root;
    std::string full_path;
    std::string index;
    struct stat fileStat;

    std::string location_path = this->location;

    document_root = locations[location_path].getRoot();
    
    this->uriIndexe = this->uri;
    std::string file_name = this->uri.substr(std::min(location_path.length(), this->uri.length()));
    
    if (document_root.empty()){
        std::cout << "EMPTY!!!\n";
        document_root = server.getRoot();
        file_name = this->uri;
    }


    if (file_name.length() > 0 && file_name[0] == '/')
        file_name = file_name.substr(1);
    if (document_root[0] != '.')
        document_root = "." + document_root;
    full_path = document_root + "/" + file_name;
    std::cout << "--------------------------> fullpath = " << full_path << std::endl;
    if (file_name.empty())
    {
        std::vector<std::string> indexes = locations[location_path].getIndex();
        if (indexes.empty())
            indexes = server.getIndex();
        for (std::vector<std::string>::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
        {
            std::string index_path = document_root;
            if (index_path[index_path.length() - 1] != '/')
                index_path += "/";
            index_path += *it;
            index = *it;
            if (stat(index_path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode))
            {
                full_path = index_path;
                this->uriIndexe += *it;
                break;
            }
        }
    }
    this->fullPath = full_path;
    if (this->fullPath.length() >= 4 && this->fullPath.substr(this->fullPath.length() - 4) == ".php")
    {
        this->cgiType = ".php";
        this->uriFileName = index;
    }
    else if (this->fullPath.length() >= 3 && this->fullPath.substr(this->fullPath.length() - 3) == ".py")
    {
        this->cgiType = ".py";
        this->uriFileName = index;
    }
    int fd = open(fullPath.c_str(), O_RDONLY);
    if (fd == -1)
    {
        this->statusCode = 404;
        return;
    }
    else
        close(fd);
}
