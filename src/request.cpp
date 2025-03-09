/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 21:20:45 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/09 14:36:30 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"

Request::Request() : statusCode(200), currentStep(REQ_LINE), state(VALID), method(UNDEFINED), reqLine(""), uri(""), httpVersion(""), body(""), isChunked(false), isBoundary(false), isContentLength(false), boundaryKey(""), contentLength(0), requestData(""), headersData(""), currentContentLength(0), fileName(""), fullBody("")
{
    this->headers["connection"] = "keep-alive";
}
Request::~Request()
{
    for (size_t i = 0; i < boundaries.size(); ++i)
        delete boundaries[i];
}

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

t_step Request::getCurrentStep() const
{
    return (this->currentStep);
}

void Request::addHeader(const std::string &key, const std::string &value)
{
    this->headers[key] = value;
}

void handleSpecialCharacters(std::string &uri)
{
    std::string encodedChars[] = {"%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27", "%28", "%29", "%2A", "%2B", "%2C",
                                  "%2D", "%2E", "%2F", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F", "%40", "%5B", "%5C", "%5D",
                                  "%5E", "%5F", "%60", "%7B", "%7C", "%7D", "%7E"};

    std::string specialChars[] = {" ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", ":", ";", "<",
                                  "=", ">", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "|", "}", "~"};

    for (size_t i = 0; i < sizeof(encodedChars) / sizeof(encodedChars[0]); i++)
    {
        std::string::size_type n = 0;
        while ((n = uri.find(encodedChars[i], n)) != std::string::npos)
        {
            uri.replace(n, encodedChars[i].length(), specialChars[i]);
            n += 1;
        }
    }
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

    handleSpecialCharacters(this->uri);
}

void Request::parseHeaders()
{
    std::istringstream headerStream(this->headersData);
    std::string line;

    while (std::getline(headerStream, line))
    {
        if (line == "\r" || line.empty())
            break;

        if (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
        {
            this->state = INVALID_HEADERS;
            this->statusCode = 400;
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

                for (size_t i = 0; i < key.size(); i++)
                    key[i] = std::tolower(key[i]);

                this->headers[key] = value;
            }
            else
            {
                this->state = INVALID_HEADERS;
                this->statusCode = 400;
                return;
            }
        }
        else
        {
            this->state = INVALID_HEADERS;
            this->statusCode = 400;
            return;
        }
    }

    if (this->headers.find("host") == this->headers.end())
    {
        this->state = HOST_MISSING;
        this->statusCode = 400;
        return;
    }
}

std::string generateRandomFileName()
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    int len = 10;
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

    std::ifstream file(tmp_s);
    if (file.good())
        return (generateRandomFileName());
    return (tmp_s);
}

void Request::parseBoundaryHeaders(const std::string &boundaryHeaders)
{
    std::istringstream headerStream(boundaryHeaders);
    std::string line, name, filename;
    std::map<std::string, std::string> heads;

    fileName = "";
    name = "";
    while (std::getline(headerStream, line))
    {
        if (line == "\r" || line.empty())
            continue;

        std::istringstream lineStream(line);
        std::string key, value;
        if (std::getline(lineStream, key, ':'))
        {
            if (std::getline(lineStream, value))
            {
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                    value.erase(0, 1);

                for (size_t i = 0; i < key.size(); i++)
                    key[i] = std::tolower(key[i]);

                heads[key] = value;
            }
        }
    }
    this->boundaries.back()->setHeaders(heads);
    for (std::map<std::string, std::string>::const_iterator it = heads.begin(); it != heads.end(); it++)
    {
        if (it->first == "content-disposition")
        {
            std::string disposition = it->second;
            size_t namePos = disposition.find("filename=");
            if (namePos != std::string::npos)
            {
                size_t start = namePos + 9;
                if (disposition[start] == '"')
                    start++;

                size_t end = disposition.find_first_of("\"", start);
                if (end != std::string::npos)
                    fileName = disposition.substr(start, end - start);
                else
                    fileName = disposition.substr(start);
            }
            namePos = disposition.find("name=");
            if (namePos != std::string::npos)
            {
                size_t start = namePos + 5;
                if (disposition[start] == '"')
                    start++;

                size_t end = disposition.find_first_of("\"", start);
                if (end != std::string::npos)
                    name = disposition.substr(start, end - start);
                else
                    name = disposition.substr(start);
            }
        }
    }
    if (!fileName.empty())
        this->boundaries.back()->setFileName(fileName);
    else if (!name.empty())
        this->boundaries.back()->setFileName(name);
    else
        this->boundaries.back()->setFileName(generateRandomFileName());
}

void Request::parseBody()
{
    if (this->isChunked)
    {
        if (this->isBoundary)
            ;
        else
            ;
    }
    else if (this->isBoundary)
    {
        static bool newFile = false;
        if (this->body.find(this->boundaryKey + "--\r\n") == 0)
        {
            this->body.clear();
            if (this->boundaries.size())
                this->boundaries.back()->closeFile();
            this->currentStep = DONE;
            return;
        }
        if (this->body.find(this->boundaryKey + "\r\n") == 0)
        {
            if (!newFile)
            {
                newFile = true;
                return;
            }
            newFile = false;
            this->body.erase(0, this->boundaryKey.size() + 2);

            if (this->boundaries.size())
                this->boundaries.back()->closeFile();
            Boundary *boundary = new Boundary();

            this->boundaries.push_back(boundary);
        }
        if (this->boundaries.size() && this->boundaries.back()->getCurrentStep() == BOUNDARY_HEADERS)
        {
            size_t pos = this->body.find("\r\n\r\n");
            if (pos != std::string::npos)
            {
                std::string boundaryHeaders = this->body.substr(0, pos);
                this->body.erase(0, pos + 4);
                this->parseBoundaryHeaders(boundaryHeaders);
                this->boundaries.back()->setCurrentStep(BOUNDARY_BODY);
            }
        }
        if (this->boundaries.size() && this->boundaries.back()->getCurrentStep() == BOUNDARY_BODY)
        {
            size_t endBoundary = this->body.find("\r\n" + this->boundaryKey);
            if (endBoundary != std::string::npos)
            {
                std::string fileContent = this->body.substr(0, endBoundary);
                this->boundaries.back()->writeToFile(fileContent);
                this->body.erase(0, endBoundary + 2);
                this->boundaries.back()->setCurrentStep(BOUNDARY_DONE);
                return;
            }
            size_t crlf = this->body.find("\r");
            while (crlf != std::string::npos)
            {            
                if (crlf == this->body.size() - 1)
                    return;
                
                if (crlf + 1 < this->body.size())
                {
                    char nextChar = this->body[crlf + 1];
                    
                    if (nextChar != '\n')
                    {
                        this->boundaries.back()->writeToFile(this->body.substr(0, crlf + 1));
                        this->body.erase(0, crlf + 1);
                    }
                    else if (crlf + 2 >= this->body.size())
                        break ;
                    else
                    {
                        char afterNewline = this->body[crlf + 2];
                        
                        if (afterNewline != '-')
                        {
                            this->boundaries.back()->writeToFile(this->body.substr(0, crlf + 2));
                            this->body.erase(0, crlf + 2);
                        }
                        else
                        {
                            if (this->body.substr(crlf + 2).size() > this->boundaryKey.size())
                            {
                                size_t isBoundaryKey = this->body.substr(crlf).find(this->boundaryKey);
                                if (isBoundaryKey != 0)
                                {
                                    this->boundaries.back()->writeToFile(this->body.substr(0, crlf + 2));
                                    this->body.erase(0, crlf + 2);
                                }
                                else
                                    break ;
                            }
                            else
                                break ;
                        }
                    }
                }

                crlf = this->body.find("\r");
            }
            if (crlf == std::string::npos && this->body.size() > this->boundaryKey.size() + 2)
            {
                size_t endBoundary = this->body.find("\r\n" + this->boundaryKey + "\r\n");
                size_t endRequest = this->body.find("\r\n" + this->boundaryKey + "--");
                if (endBoundary == std::string::npos && endRequest == std::string::npos)
                {
                    this->boundaries.back()->writeToFile(this->body);
                    this->body.clear();
                }
            }
        }
    }
    else if (this->isContentLength)
    {
        if (this->contentLength == this->currentContentLength)
        {
            std::ofstream newfile("./video.mp4", std::ios::out | std::ios::binary | std::ios::app);
            newfile.write(this->body.c_str(), this->body.size());
            this->body.clear();
            this->currentStep = DONE;
        }
    }
    else
    {
        this->statusCode = 400;
        this->state = INVALID_HEADERS;
    }
}

void Request::setBodyInformations()
{
    std::map<std::string, std::string>::iterator transferEncodingIt = this->headers.find("transfer-encoding");
    if (transferEncodingIt != this->headers.end())
    {
        if (transferEncodingIt->second != "chunked")
        {
            this->state = INVALID_HEADERS;
            this->statusCode = 400;
            return;
        }
        this->isChunked = true;
    }

    std::map<std::string, std::string>::iterator contentLengthIt = this->headers.find("content-length");
    if (contentLengthIt != this->headers.end())
    {
        char *end;
        this->contentLength = strtol(contentLengthIt->second.c_str(), &end, 10);
        if (contentLength < 0)
        {
            this->state = INVALID_HEADERS;
            this->statusCode = 400;
            return;
        }
        this->isContentLength = true;
    }

    std::map<std::string, std::string>::iterator contentTypeIt = this->headers.find("content-type");
    if (contentTypeIt != this->headers.end())
    {
        std::string contentType = contentTypeIt->second;
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos)
        {
            this->boundaryKey = "--" + contentType.substr(boundaryPos + 9);
            this->isBoundary = true;
            this->boundaryKey.erase(this->boundaryKey.size() - 1);
        }
    }
}

void Request::parseRequest(const std::string &rawRequest)
{
    if (this->state != VALID || this->currentStep == DONE)
        return;
    this->requestData += rawRequest;
    if (this->state == VALID && this->currentStep == REQ_LINE)
    {
        size_t pos = this->requestData.find("\r\n");
        if (pos != std::string::npos)
        {
            this->reqLine = this->requestData.substr(0, pos);
            this->requestData.erase(0, pos + 2);
            this->parseRequestLine();
            if (this->state != VALID)
                return;
            this->currentStep = HEADERS;
        }
    }
    if (this->state == VALID && this->currentStep == HEADERS)
    {
        size_t pos = this->requestData.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            this->headersData = this->requestData.substr(0, pos);
            this->requestData.erase(0, pos + 4);
            this->parseHeaders();
            if (this->state != VALID)
                return;
            this->setBodyInformations();
            this->currentStep = BODY;
        }
    }
    if (this->state == VALID && this->currentStep == BODY)
    {
        this->fullBody += this->requestData;
        this->body += this->requestData;
        this->currentContentLength += requestData.size();
        this->requestData.clear();
        this->parseBody();
        if (this->state != VALID)
            return;
    }
}

void Request::printRequest()
{
    if (this->currentStep != DONE)
        return;
    std::cout << this->reqLine << std::endl
              << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); it++)
        std::cout << it->first << ": " << it->second << std::endl;
    std::cout << std::endl;
    if (!this->requestData.empty())
        std::cout << this->requestData << std::endl;
}
