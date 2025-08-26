/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-request_line.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 07:42:10 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/24 19:52:13 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "WebServer.hpp"

void Request::parseRequestLine()
{
    if (!this->reqLine.empty() && (this->reqLine[0] == ' ' || this->reqLine[0] == '\t'))
    {
        this->message = "Invalid request line";
        throw 400;
    }

    std::istringstream lineStream(this->reqLine);
    std::string methodStr;

    if (!(lineStream >> methodStr >> this->uri >> this->httpVersion))
    {
        this->message = "Invalid request line";
        throw 400;
    }

    if (methodStr == "GET")
        this->method = GET;
    else if (methodStr == "POST")
        this->method = POST;
    else if (methodStr == "DELETE")
        this->method = DELETE;
    else
    {
        this->message = "Invalid Method";
        throw 400;
    }

    size_t pos = this->uri.find('?');

    if (pos != std::string::npos)
    {
        this->uriQueries = this->uri.substr(pos + 1);
        this->uri = this->uri.substr(0, pos);
    }

    size_t lastSlash = this->uri.find_last_of('/');
    std::string lastPart;
    if (lastSlash != std::string::npos && lastSlash + 1 < this->uri.size())
    {
        lastPart = this->uri.substr(lastSlash + 1);
    }

    if (lastPart.empty())
        this->uriFileName = "";
    else if (lastPart.size() > 3 && lastPart.substr(lastPart.size() - 3) == ".py")
        this->uriFileName = lastPart;
    else if (lastPart.size() > 4 && lastPart.substr(lastPart.size() - 4) == ".php")
        this->uriFileName = lastPart;
    else if (!this->uriQueries.empty())
        this->uriFileName = lastPart;
    else
        this->uriFileName = "";

    handleUriSpecialCharacters(this->uri);

    const ServerConf &server = globalServer[0];

    std::map<std::string, LocationConf> locations = server.getLocations();

    std::string matchedRoute = "";
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        const std::string &route = it->first;
        if (this->uri == route ||
            (this->uri.find(route) == 0 && route != "/" &&
             (route[route.size() - 1] == '/' || this->uri[route.length()] == '/')))
        //  (route.back() == '/' || this->uri[route.length()] == '/')))
        {
            if (route.length() > matchedRoute.length())
            {
                matchedRoute = route;
            }
        }
    }

    if (matchedRoute.empty() && locations.find("/") != locations.end())
    {
        matchedRoute = "/";
    }

    this->location = matchedRoute;

    if (this->uri[0] != '/')
    {
        this->message = "Invalid URI";
        throw 400;
    }

    if (this->uri.length() >= 4 && this->uri.substr(this->uri.length() - 4) == ".php")
        this->cgiType = ".php";
    else if (this->uri.length() >= 3 && this->uri.substr(this->uri.length() - 3) == ".py")
        this->cgiType = ".py";

    if (this->isCGI())
    {
        std::string filename = generateRandomFileName("./");
        this->createdFile = filename;
        int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd == -1)
        {
            this->message = "Failed to open temporary file";
            throw 500;
        }

        if (unlink(filename.c_str()) == -1)
        {
            close(fd);
            this->message = "Failed to unlink temporary file";
            throw 500;
        }

        int fd2 = dup(fd);
        if (fd2 == -1)
        {
            close(fd);
            this->message = "Failed to duplicate file descriptor";
            throw 500;
        }

        this->cgiFdRead = fd;
        this->cgiFdWrite = fd2;
    }

    if (this->httpVersion.size() < 6 || this->httpVersion.substr(0, 5) != "HTTP/")
    {
        this->message = "Invalid request line";
        throw 400;
    }

    std::string version = this->httpVersion.substr(5);

    if (version.empty() || version.find_first_not_of("0123456789.") != std::string::npos)
    {
        this->message = "Invalid request line";
        throw 400;
    }

    double versionNumber = atof(version.c_str());
    if (versionNumber < 1.0)
    {
        this->message = "Invalid request line";
        throw 400;
    }

    if (this->httpVersion != "HTTP/1.1")
    {
        this->message = "HTTP Version Not Supported";
        throw 505;
    }

    std::string extra;
    if (lineStream >> extra)
    {
        this->message = "Invalid request line";
        throw 400;
    }
}
