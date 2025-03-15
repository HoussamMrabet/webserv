/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 21:20:45 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/15 07:17:23 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : statusCode(200), message("success!"), currentStep(REQ_LINE), method(UNDEFINED), reqLine(""), uri(""), httpVersion(""), body(""), isChunked(false), isMultipart(false), isContentLength(false), boundaryKey(""), contentLength(0), requestData(""), headersData(""), currentContentLength(0), fileName(""), fullBody(""), chunkSize(0), chunkData(""), inChunk(false)
{
    this->headers["connection"] = "keep-alive";
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

int Request::getStatusCode() const
{
    return (this->statusCode);
}

std::string Request::getUri() const
{
    return (this->uri);
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

t_step Request::getCurrentStep() const
{
    return (this->currentStep);
}

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
    
    handleUriSpecialCharacters(this->uri);
    if (this->uri[0] != '/')
    {
        this->message = "Invalid URI";
        throw 400;
    }
    
    if (httpVersion.size() < 6 || httpVersion.substr(0, 5) != "HTTP/")
    {
        this->message = "Invalid request line";
        throw 400;
    }

    std::string version = httpVersion.substr(5);

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

    if (httpVersion != "HTTP/1.1")
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
            this->message = "Invalid header: `" + line + "`";
            throw 400;
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
                    this->message = "Invalid header: `" + lineStream.str() + "`";
                    throw 400;
                }

                for (size_t i = 0; i < key.size(); i++)
                    key[i] = std::tolower(key[i]);

                this->headers[key] = value.back() == '\r' ? value.substr(0, value.size() - 1) : value;
            }
            else
                this->headers[key] = "";
        }
    }

    if (this->headers.find("host") == this->headers.end())
    {
        this->message = "Invalid headers: Host is missing";
        throw 400;
    }
}

void Request::parseMultipartHeaders(const std::string &multipartHeaders)
{
    std::istringstream headerStream(multipartHeaders);
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
    this->multipartData.back()->setHeaders(heads);
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
        else if (it->first == "content-type")
        {
            std::string contentType = it->second;
            std::transform(contentType.begin(), contentType.end(), contentType.begin(), ::tolower);
            this->multipartData.back()->setContentType(contentType);
        }
    }
    if (!fileName.empty())
        this->multipartData.back()->setFileName(fileName);
    else if (!name.empty())
        this->multipartData.back()->setFileName(name);
    else
        this->multipartData.back()->setFileName(generateRandomFileName());
}

void Request::parseMultipart(bool isChunked)
{
    static bool newFile = false;
    std::string &data = isChunked ? this->chunkData : this->body;
    
    if (data.find(this->boundaryKey + "--\r\n") == 0)
    {
        data.clear();
        if (this->multipartData.size())
            this->multipartData.back()->closeFile();
        throw 200;
    }
    if (data.find(this->boundaryKey + "\r\n") == 0)
    {
        if (!newFile)
        {
            newFile = true;
            return ;
        }
        newFile = false;
        data.erase(0, this->boundaryKey.size() + 2);

        if (this->multipartData.size())
            this->multipartData.back()->closeFile();
        Multipart *multipart = new Multipart();

        this->multipartData.push_back(multipart);
    }
    if (this->multipartData.size() && this->multipartData.back()->getCurrentStep() == MULTIPART_HEADERS)
    {
        size_t pos = data.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            std::string multipartHeaders = data.substr(0, pos);
            data.erase(0, pos + 4);
            this->parseMultipartHeaders(multipartHeaders);
            this->multipartData.back()->setCurrentStep(MULTIPART_BODY);
        }
    }
    if (this->multipartData.size() && this->multipartData.back()->getCurrentStep() == MULTIPART_BODY)
    {
        size_t endBoundary = data.find("\r\n" + this->boundaryKey);
        if (endBoundary != std::string::npos)
        {
            std::string fileContent = data.substr(0, endBoundary);
            this->multipartData.back()->writeToFile(fileContent);
            data.erase(0, endBoundary + 2);
            this->multipartData.back()->setCurrentStep(MULTIPART_DONE);
            return;
        }
        size_t crlf = data.find("\r");
        while (crlf != std::string::npos)
        {
            if (crlf == data.size() - 1)
                return;

            if (crlf + 1 < data.size())
            {
                char nextChar = data[crlf + 1];

                if (nextChar != '\n')
                {
                    this->multipartData.back()->writeToFile(data.substr(0, crlf + 1));
                    data.erase(0, crlf + 1);
                }
                else if (crlf + 2 >= data.size())
                    break;
                else
                {
                    char afterNewline = data[crlf + 2];

                    if (afterNewline != '-')
                    {
                        this->multipartData.back()->writeToFile(data.substr(0, crlf + 2));
                        data.erase(0, crlf + 2);
                    }
                    else
                    {
                        if (data.substr(crlf + 2).size() > this->boundaryKey.size())
                        {
                            size_t isBoundaryKey = data.substr(crlf).find(this->boundaryKey);
                            if (isBoundaryKey != 0)
                            {
                                this->multipartData.back()->writeToFile(data.substr(0, crlf + 2));
                                data.erase(0, crlf + 2);
                            }
                            else
                                break;
                        }
                        else
                            break;
                    }
                }
            }

            crlf = data.find("\r");
        }
        if (crlf == std::string::npos && data.size() > this->boundaryKey.size() + 2)
        {
            size_t endBoundary = data.find("\r\n" + this->boundaryKey + "\r\n");
            size_t endRequest = data.find("\r\n" + this->boundaryKey + "--");
            if (endBoundary == std::string::npos && endRequest == std::string::npos)
            {
                this->multipartData.back()->writeToFile(data);
                data.clear();
            }
        }
    }
}

void Request::parseBody()
{
    if (this->isChunked)
    {
        if (this->body.empty())
            parseMultipart(CHUNKED);
        if (inChunk)
        {
            if (this->body.size() < this->chunkSize + 2)
                return ;
            else
            {
                if (this->isMultipart)
                {
                    this->chunkData += this->body.substr(0, this->chunkSize);
                    parseMultipart(CHUNKED);
                    this->body.erase(0, this->chunkSize + 2);
                }
                else
                {
                    this->file.write(this->body.substr(0, this->chunkSize).c_str(), this->chunkSize);
                    this->file.flush();
                    this->body.erase(0, this->chunkSize + 2);
                }
                inChunk = false;
            }
        }
        size_t pos = this->body.find("\r\n");
        if (pos != std::string::npos)
        {
            std::string chunkSizeStr = this->body.substr(0, pos);
            this->body.erase(0, pos + 2);
            char *end;
            this->chunkSize = strtol(chunkSizeStr.c_str(), &end, 16);
            this->inChunk = true;
            if (this->chunkSize < 0)
            {
                this->message = "Invalid chunk size";
                throw 400;
            }
            if (this->chunkSize == 0)
            {
                this->body.clear();
                throw 200;
            }
        }
    }
    else if (this->isMultipart)
    {
        parseMultipart();
    }
    else if (this->isContentLength)
    {
        if (this->contentLength == this->currentContentLength)
        {
            this->file.write(this->body.c_str(), this->body.size());
            this->file.flush();
            this->file.close();
            this->body.clear();
            throw 200;
        }
    }
    else
    {
        this->message = "Invalid headers: Content-Length is missing";
        throw 400;
    }
}

void Request::setBodyInformations()
{
    std::map<std::string, std::string>::iterator transferEncodingIt = this->headers.find("transfer-encoding");
    if (transferEncodingIt != this->headers.end())
    {
        std::string transferEncoding = transferEncodingIt->second;
        std::transform(transferEncoding.begin(), transferEncoding.end(), transferEncoding.begin(), ::tolower);
        if (transferEncoding != "chunked")
        {
            this->message = "Unsupported Transfer-Encoding";
            throw 501;
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
            this->message = "Invalid Content-Length";
            throw 400;
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
            this->isMultipart = true;
            this->boundaryKey.erase(this->boundaryKey.size());
        }
    }
    if (!this->isMultipart)
    {
        this->file = std::ofstream(generateRandomFileName("./binary_file_"), std::ios::out | std::ios::binary | std::ios::trunc);
        if (!this->file.is_open())
        {
            this->message = "Failed to open file";
            throw 500;
        }
    }
}

void Request::parseRequest(const std::string &rawRequest)
{    
    if (this->statusCode != 200 || this->currentStep == DONE)
        return ;
    try
    {
        this->requestData += rawRequest;
        if (this->currentStep == REQ_LINE)
        {
            size_t pos = this->requestData.find("\r\n");
            if (pos != std::string::npos)
            {
                this->reqLine = this->requestData.substr(0, pos);
                this->requestData.erase(0, pos + 2);
                this->parseRequestLine();
                this->currentStep = HEADERS;
            }
        }
        if (this->currentStep == HEADERS)
        {
            size_t pos = this->requestData.find("\r\n\r\n");
            if (pos != std::string::npos)
            {
                this->headersData = this->requestData.substr(0, pos);
                this->requestData.erase(0, pos + 4);
                this->parseHeaders();
                this->setBodyInformations();
                this->currentStep = BODY;
            }
        }
        if (this->currentStep == BODY)
        {
            this->fullBody += this->requestData;
            this->body += this->requestData;
            this->currentContentLength += requestData.size();
            this->requestData.clear();
            this->parseBody();
        }
    }
    catch(const int &e)
    {
        this->statusCode = e;
        this->currentStep = DONE;
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
