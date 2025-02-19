/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 21:20:45 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/19 08:06:02 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"

Request::Request() : statusCode(200), currentStep(REQ_LINE), state(VALID), method(UNDEFINED), reqLine(""), uri(""), httpVersion(""), body("")
{
    this->headers["Connection"] = "Keep-Alive";
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
        return;
    }

    std::istringstream lineStream(this->reqLine);
    std::string methodStr;

    if (!(lineStream >> methodStr >> this->uri >> this->httpVersion))
    {
        this->state = INVALID_REQ_LINE;
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

    if (httpVersion != "HTTP/1.0" && httpVersion != "HTTP/1.1")
    {
        this->state = INVALID_REQ_LINE;
        return;
    }

    this->state = VALID;
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
                headers[lastHeaderKey] += " " + line.substr(1);
            else
                state = INVALID_HEADERS;
            continue;
        }

        std::istringstream lineStream(line);
        std::string key, value;
        if (std::getline(lineStream, key, ':'))
        {
            if (std::getline(lineStream, value))
            {
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                    value.erase(0, 1);

                headers[key] = value;
                lastHeaderKey = key;
            }
            else
            {
                state = INVALID_HEADERS;
                return;
            }
        }
        else
        {
            state = INVALID_HEADERS;
            return;
        }
    }

    if (httpVersion >= "HTTP/1.1" && headers.find("Host") == headers.end())
    {
        state = HOST_MISSING;
        return;
    }

    state = VALID;
}


void Request::parseRequest(const std::string &rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

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
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
        std::cout << it->first << ": " << it->second << std::endl;
    if (!this->body.empty())
        std::cout << "\nBody\n"
                  << body << std::endl;
}
