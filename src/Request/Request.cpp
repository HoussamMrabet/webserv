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
#include "WebServ.hpp"

std::string Request::theme = "light";
std::vector<t_user> Request::users;
t_user Request::loggedInUser;
bool Request::loggedIn = false;


Request::Request() : statusCode(200), message("success!"), currentStep(REQ_LINE),
                        method(UNDEFINED), reqLine(""), uri(""), uriQueries(""), uriFileName(""),
                        location(""), cgiType(""), host(""), httpVersion(""), body(""),file(-1),
                        isChunked(false), isMultipart(false), isContentLength(false), boundaryKey(""),
                        contentLength(0), requestData(""), headersData(""), currentContentLength(0),
                        fileName(""), createdFile(""), fullBody(""), chunkSize(0), chunkData(""), inChunk(false),
                        cgiFdRead(-1), cgiFdWrite(-1), server(ConfigBuilder::getServer())
{
    CHOROUK && std::cout << G"-------- Request destructor called!! ----";
    CHOROUK && std::cout << B"\n";
    this->headers["connection"] = "keep-alive";
    // const ServerConf &server = globalServer[0];
    // const ServerConf &server = ConfigBuilder::getServer();
    this->uploadDir = server.getRoot() + server.getUploadDir();
}

Request::~Request()
{
    CHOROUK && std::cout << G"-------- Request destructor called!! ----";
    CHOROUK && std::cout << B"\n";
    for (size_t i = 0; i < multipartData.size(); ++i)
        delete multipartData[i];
    if (this->file != -1)
    {
        close(this->file);
        this->file = -1;
    }
}

std::string Request::getMessage() const
{
    return (this->message);
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

void Request::setStatusCode(int n)
{
    this->statusCode = n;
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

std::string Request::getRoot() const
{
    return (this->root);
}

std::string Request::getFullPath() const
{
    return (this->fullPath);
}

std::string Request::getFullUri() const
{
    return (this->uriIndexe);
}

bool Request::isDone() const
{
    return (this->currentStep == DONE);
}

bool Request::isCGI() const
{
    return !this->cgiType.empty();
}

void Request::CGIError()
{
    this->cgiType = "";
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
    // std::cout << Request::loggedInUser.username << std::endl;
    // std::cout << Request::loggedInUser.password << std::endl;

    // std::cout << "loggedin : " << Request::loggedIn << std::endl;
    // if (!this->fullBody.empty())
    //     std::cout << this->fullBody << std::endl;
    //     std::cout << "----------------------------------------" << std::endl;
}

size_t Request::getBodySizeLimit() const
{
    return (this->bodySizeLimit);
}
