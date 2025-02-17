/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 21:20:45 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/18 00:41:58 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"

Request::Request() {}
Request::~Request() {}

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
    this->headers.clear();
    this->headers.insert(headers.begin(), headers.end());
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

void Request::parseRequest(const std::string &rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    // request line
    if (std::getline(requestStream, line))
    {
        std::istringstream lineStream(line);
        lineStream >> this->method >> this->uri >> this->httpVersion;
    }

    // headers
    while (std::getline(requestStream, line) && line != "\r")
    {
        std::istringstream headerStream(line);
        std::string key, value;
        if (std::getline(headerStream, key, ':'))
        {
            if (std::getline(headerStream, value))
            {
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                    value.erase(0, 1);
                headers[key] = value;
            }
        }
    }

    if (headers.find("Content-Length") != headers.end())
    {
        std::istringstream contentLengthStream(headers["Content-Length"]);
        int contentLength;
        contentLengthStream >> contentLength;
        char *buffer = new char[contentLength + 1];
        requestStream.read(buffer, contentLength);
        buffer[contentLength] = '\0';
        body = buffer;
        delete[] buffer;
    }
    else if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked")
    {
        // "0\r\n\r\n"
        body.clear();
        while (std::getline(requestStream, line))
        {
            int chunkSize;
            std::istringstream chunkSizeStream(line);
            chunkSizeStream >> std::hex >> chunkSize;
            if (chunkSize == 0)
                break;
            char *buffer = new char[chunkSize + 1];
            requestStream.read(buffer, chunkSize);
            buffer[chunkSize] = '\0';
            body += buffer;
            delete[] buffer;
            requestStream.ignore(2); // \r\n
        }
    }
    else if (headers.find("Content-Type") != headers.end() && headers["Content-Type"].find("multipart/form-data") != std::string::npos)
    {
        std::string boundary = "--" + headers["Content-Type"].substr(headers["Content-Type"].find("boundary=") + 9);
        std::string fullBody((std::istreambuf_iterator<char>(requestStream)), std::istreambuf_iterator<char>());

        size_t pos = 0;
        while ((pos = fullBody.find(boundary)) != std::string::npos)
        {
            std::string part = fullBody.substr(0, pos);
            body += part + "\n";
            fullBody.erase(0, pos + boundary.length());
        }
    }
}

void Request::printRequest() const
{
    std::cout << this->method << " " << this->uri << " " << this->httpVersion << std::endl;
    std::cout << "Header" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
        std::cout << it->first << ": " << it->second << std::endl;
    if (!this->body.empty())
        std::cout << "\nBody\n"
                  << body << std::endl;
}
