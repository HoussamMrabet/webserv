/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-multipart.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 07:46:58 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/22 03:24:20 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

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
            try
            {
                checkMediaType(contentType);
            }
            catch(const char *e)
            {
                this->message = e;
                throw 415;
            }
        }
    }
    try
    {
        if (!fileName.empty())
            this->multipartData.back()->setFileName(this->uploadDir + "/" + fileName);
        else if (!name.empty())
            this->multipartData.back()->setFileName(this->uploadDir + "/" + name);
        else
            this->multipartData.back()->setFileName(this->uploadDir + "/" + generateRandomFileName());
    }
    catch(const char *e)
    {
        this->message = e;
        throw 500;
    }
    
}

void Request::parseMultipart(bool isChunked)
{
    static bool newFile = false;
    std::string &data = isChunked ? this->chunkData : this->body;

    std::cout << "parseMultipart(" << (isChunked ? "CHUNKED" : "NORMAL") << ") - data.size=" << data.size() << "\n";
    if (data.size() > 0 && data.size() < 200)
        std::cout << "  First 200 bytes: [" << data.substr(0, std::min(data.size(), (size_t)200)) << "]\n";

    try
    {
        if (data.find(this->boundaryKey + "--\r\n") == 0)
        {
            std::cout << "  Found closing boundary! Throwing 200\n";
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
            
            size_t crlf;
            while ((crlf = data.find("\r")) != std::string::npos && crlf + 1 < data.size())
            {
                if (data[crlf + 1] != '\n')
                {
                    this->multipartData.back()->writeToFile(data.substr(0, crlf + 1));
                    data.erase(0, crlf + 1);
                    continue;
                }
            
                if (crlf + 2 >= data.size())
                    break;
            
                if (data[crlf + 2] != '-')
                {
                    this->multipartData.back()->writeToFile(data.substr(0, crlf + 2));
                    data.erase(0, crlf + 2);
                    continue;
                }
            
                if (data.substr(crlf + 2).size() <= this->boundaryKey.size() || data.substr(crlf).find(this->boundaryKey) == 0)
                    break;
            
                this->multipartData.back()->writeToFile(data.substr(0, crlf + 2));
                data.erase(0, crlf + 2);
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
    catch(const char *e)
    {
        this->message = e;
        throw 500;
    }
}
