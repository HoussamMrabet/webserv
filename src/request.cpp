/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 21:20:45 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/19 17:23:22 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"

Request::Request() : statusCode(200), currentStep(REQ_LINE), state(VALID), method(UNDEFINED), reqLine(""), uri(""), httpVersion(""), body(""), isChunked(false), isBoundary(false), isContentLength(false), contentLength(0), boundaryKey("")
{
    this->headers["Connection"] = "close";
}
Request::~Request() {}

void Request::setReqLine(const std::string &reqLine)
{
    this->reqLine = reqLine;
}

void Request::setMethod(t_method &method)
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

std::string Request::getReqLine() const
{
    return (this->reqLine);
}

t_method Request::getMethod() const
{
    return (this->method);
}

t_reqState Request::getState() const
{
    return (this->state);
}

int Request::getStatusCode() const
{
    return (this->statusCode);
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

void Request::parseRequestLine()
{
    if (!this->reqLine.empty() && (this->reqLine[0] == ' ' || this->reqLine[0] == '\t'))
    {
        this->state = INVALID_REQ_LINE;
        this->statusCode = 400;
        return;
    }

    std::istringstream lineStream(this->reqLine);
    std::string methodStr;

    if (!(lineStream >> methodStr >> this->uri >> this->httpVersion))
    {
        this->state = INVALID_REQ_LINE;
        this->statusCode = 400;
        return;
    }

    if (methodStr == "GET")
        this->method = GET;
    else if (methodStr == "POST")
        this->method = POST;
    else if (methodStr == "DELETE")
        this->method = DELETE;
    else
    {
        this->statusCode = 400;
        this->method = INVALID;
        this->state = INVALID_METHOD;
        return;
    }

    if (httpVersion.size() < 6 || httpVersion.substr(0, 5) != "HTTP/")
    {
        this->state = INVALID_REQ_LINE;
        this->statusCode = 400;
        return;
    }

    std::string version = httpVersion.substr(5);

    if (version.empty() || version.find_first_not_of("0123456789.") != std::string::npos)
    {
        this->state = INVALID_REQ_LINE;
        this->statusCode = 400;
        return;
    }

    double versionNumber = atof(version.c_str());
    if (versionNumber < 1.0 || versionNumber > 1.999)
    {
        this->state = INVALID_REQ_LINE;
        this->statusCode = 400;
        return;
    }

    if (httpVersion != "HTTP/1.1")
    {
        this->state = INVALID_REQ_LINE;
        this->statusCode = 505;
        return;
    }
}

void Request::parseHeaders()
{
    std::istringstream headerStream(reqLine);
    std::string line;
    std::string lastHeaderKey;

    while (std::getline(headerStream, line))
    {
        if (line == "\r" || line.empty())
            break;

        if (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
        {
            if (!lastHeaderKey.empty())
                this->headers[lastHeaderKey] += " " + line.substr(1);
            else
            {
                this->state = INVALID_HEADERS;
                this->statusCode = 400;
            }
            return;
        }

        std::istringstream lineStream(line);
        std::string key, value;
        if (std::getline(lineStream, key, ':'))
        {
            if (std::getline(lineStream, value))
            {
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                    value.erase(0, 1);

                if (!key.empty() && (key[0] == ' ' || key[0] == '\t' || key[key.size() - 1] == ' ' || key[key.size() - 1] == '\t'))
                {
                    this->state = INVALID_HEADERS;
                    this->statusCode = 400;
                    return;
                }
                this->headers[key] = value;
                lastHeaderKey = key;
            }
            else
            {
                this->state = INVALID_HEADERS;
                return;
            }
        }
        else
        {
            this->state = INVALID_HEADERS;
            return;
        }
    }

    bool hostFound = false;
    for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); ++it)
    {
        std::string key = it->first;

        for (size_t i = 0; i < key.length(); i++)
            key[i] = std::tolower(key[i]);

        if (key == "host")
        {
            hostFound = true;
            break;
        }
    }

    if (!hostFound)
    {
        this->state = HOST_MISSING;
        this->statusCode = 400;
        return;
    }
}

void Request::parseRequest(const std::string &rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    if (this->state != VALID)
        return;
    while (std::getline(requestStream, line))
    {
        if (this->currentStep == REQ_LINE)
        {
            this->reqLine += line;
            if (this->reqLine.find("\r") != std::string::npos)
            {
                this->parseRequestLine();
                if (this->state != VALID)
                    return;
                this->currentStep = HEADERS;
            }
        }
        else if (this->currentStep == HEADERS)
        {
            if (line == "\r")
            {
                this->parseHeaders();
                if (this->state != VALID)
                    return;

                std::map<std::string, std::string>::iterator transferEncodingIt = this->headers.find("Transfer-Encoding");
                if (transferEncodingIt != headers.end())
                {
                    if (transferEncodingIt->second != "chunked")
                    {
                        this->state = INVALID_HEADERS;
                        this->statusCode = 400;
                        return;
                    }
                    this->isChunked = true;
                }

                std::map<std::string, std::string>::iterator contentLengthIt = this->headers.find("Content-Length");
                if (contentLengthIt != headers.end())
                {
                    char *end;
                    contentLength = strtol(contentLengthIt->second.c_str(), &end, 10);
                    if (*end != '\0' || contentLength < 0)
                    {
                        this->state = INVALID_HEADERS;
                        this->statusCode = 400;
                        return;
                    }
                    this->isContentLength = true;
                }

                std::map<std::string, std::string>::iterator contentTypeIt = this->headers.find("Content-Type");
                if (contentTypeIt != headers.end())
                {
                    std::string contentType = contentTypeIt->second;
                    size_t boundaryPos = contentType.find("boundary=");
                    if (boundaryPos != std::string::npos)
                    {
                        this->boundaryKey = contentType.substr(boundaryPos + 9);
                        this->isBoundary = true;
                    }
                }

                this->currentStep = BODY;
            }
            else
            {
                this->headers[line.substr(0, line.find(":"))] = line.substr(line.find(":") + 2);
            }
        }
        else if (this->currentStep == BODY)
        {
            this->body += line;
            this->parseBody();
        }
    }
}

void Request::printRequest() const
{
    std::cout << this->reqLine << std::endl;
    std::cout << "Header" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); it++)
        std::cout << it->first << ": " << it->second << std::endl;
    if (!this->body.empty())
        std::cout << "\nBody\n" << this->body << std::endl;
}
