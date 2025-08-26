/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 21:20:45 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/24 20:56:08 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "WebServer.hpp"

Request::Request() : statusCode(200), message("success!"), currentStep(REQ_LINE), uriFileName(""),
                        method(UNDEFINED), reqLine(""), uri(""), uriQueries(""), cgiType(""),
                        httpVersion(""), body(""), isChunked(false), isMultipart(false),
                        isContentLength(false), boundaryKey(""), contentLength(0),
                        requestData(""), headersData(""), currentContentLength(0),
                        fileName(""), fullBody(""), chunkSize(0), chunkData(""), inChunk(false),
                        cgiFdRead(-1), cgiFdWrite(-1)
{
    this->headers["connection"] = "keep-alive";
    const ServerConf &server = globalServer[0];
    this->uploadDir = server.getUploadDir();
}
Request::~Request()
{
    for (size_t i = 0; i < multipartData.size(); ++i)
        delete multipartData[i];
    if (file.is_open())
        file.close();
}

t_method Request::getMethod() const
{
    return (this->method);
}

std::string Request::getStrMethod() const
{
    switch (this->method)
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

int Request::getStatusCode() const
{
    return (this->statusCode);
}

std::string Request::getUri() const
{
    return (this->uri);
}

std::string Request::getUriQueries() const
{
    return (this->uriQueries);
}

std::string Request::getUriFileName() const
{
    return (this->uriFileName);
}

std::string Request::getLocation() const
{
    return (this->location);
}

std::map<std::string, std::string> Request::getHeaders() const
{
    return (this->headers);
}

std::string Request::getBody() const
{
    return (this->fullBody);
}

std::string Request::getHeader(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = this->headers.find(key);
    if (it != this->headers.end())
    return it->second;
    return "";
}

std::string Request::getHost() const
{
    return (getHeader("host"));
}

std::string Request::getCgiType() const
{
    return (this->cgiType);
}

int Request::getCgiFdRead() const
{
    return cgiFdRead;
}

bool Request::isDone() const
{
    return (this->currentStep == DONE);
}

bool Request::isCGI() const
{
    return !this->cgiType.empty();
}

void Request::printRequest()
{
    if (!isDone())
        return ;
    std::cout << this->reqLine << std::endl
              << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); it++)
        std::cout << it->first << ": " << it->second << std::endl;
    std::cout << std::endl;
    if (!this->fullBody.empty())
        std::cout << this->fullBody << std::endl;
}
