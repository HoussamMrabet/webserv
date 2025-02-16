/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 21:20:45 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/16 22:15:08 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"

Request::Request(){}
Request::~Request(){}

void Request::setMethod(const std::string &method)
{
    this->method = method;
}

void Request::setUri(const std::string &uri)
{
    this->uri = uri;
}

void Request::setHttpVersion(const std::string &httpVersion)
{
    this->httpVersion = httpVersion;
}

void Request::setHeaders(const std::map<std::string, std::string> &headers)
{
    this->headers = headers;
}

void Request::setBody(const std::string &body)
{
    this->body = body;
}

std::string Request::getMethod() const
{
    return (this->method);
}

std::string Request::getUri() const
{
    return (this->uri);
}

std::string Request::getHttpVersion() const
{
    return (this->httpVersion);
}

std::map<std::string, std::string> Request::getHeaders() const
{
    return (this->headers);
}

std::string Request::getBody() const
{
    return (this->body);
}

std::string Request::getHeader(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = this->headers.find(key);
    if (it != this->headers.end())
        return it->second;
    return "";
}
void Request::addHeader(const std::string &key, const std::string &value)
{
    this->headers[key] = value;
}

void Request::printRequest() const {
    std::cout << this->method << " " << this->uri << " " << this->httpVersion << std::endl;
    std::cout << "Header" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
        std::cout << it->first << ": " << it->second << std::endl;
    if (!this->body.empty())
        std::cout << "\nBody\n" << body << std::endl;
}
