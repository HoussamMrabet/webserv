/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-body.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 07:48:15 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/24 16:39:32 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void Request::parseBody()
{
    if (this->isChunked)
    {
        if (!this->lastChunk && this->body.size() >= 5)
        {
            std::string ending = this->body.substr(this->body.size() - 5);
            if (ending == "0\r\n\r\n")
            {
                this->lastChunk = true;
                std::cout << "LAST CHUNK DETECTED IN BODY\n";
            }
        }
        
        if (this->body.empty())
            parseMultipart(CHUNKED);
        if (inChunk)
        {
            if (this->body.size() < this->chunkSize + 2)
                return;
            else
            {
                if (this->isMultipart && !this->isCGI())
                {
                    this->chunkData += this->body.substr(0, this->chunkSize);
                    parseMultipart(CHUNKED);
                    this->body.erase(0, this->chunkSize + 2);
                }
                else
                {
                    if (this->isCGI())
                    {
                        ssize_t written = write(this->cgiFdWrite, this->body.c_str(), this->chunkSize + 2);
                        if (written == -1)
                        {
                            this->message = "Failed to write to CGI pipe";
                            throw 500;
                        }
                    }
                    else
                    {
                        std::string chunk = this->body.substr(0, this->chunkSize);

                        if (this->headers.find("content-type") != this->headers.end())
                        {
                            std::string contentType = this->headers["content-type"];

                            try
                            {
                                checkMediaType(contentType);
                            }
                            catch (const char *error)
                            {
                                this->message = error;
                                throw 415;
                            }
                            ssize_t written = write(this->file, chunk.c_str(), this->chunkSize);
                            if (written == -1)
                            {
                                this->message = "Failed to write to file";
                                throw 500;
                            }
                        }
                    }
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
            if ((signed)this->chunkSize < 0)
            {
                this->message = "Invalid chunk size";
                throw 400;
            }
            if (this->chunkSize == 0)
            {
                std::cout << "LAST CHUNK RECEIVED\n";
                this->body.clear();
                throw 200;
            }
        }
    }
    else if (this->isMultipart && !this->isCGI())
    {
        parseMultipart();
        if (currentContentLength >= contentLength)
        {
            while (true)
                parseMultipart();
        }
    }
    else if (this->isContentLength || this->isCGI())
    {
        if (this->contentLength == this->currentContentLength)
        {
            if (this->isCGI())
            {
                ssize_t written = write(this->cgiFdWrite, this->body.c_str(), this->body.size());
                if (written == -1)
                {
                    this->message = "Failed to write to CGI pipe";
                    throw 500;
                }
                close(this->cgiFdWrite);
            }
            else
            {
                if (this->headers.find("content-type") != this->headers.end())
                {
                    std::string contentType = this->headers["content-type"];

                    try
                    {
                        checkMediaType(contentType);
                    }
                    catch (const char *error)
                    {
                        this->message = error;
                        throw 415;
                    }
                    ssize_t written = write(this->file, this->body.c_str(), this->body.size());
                    if (written == -1)
                    {
                        this->message = "Failed to write to file";
                        throw 500;
                    }

                    if (close(this->file) == -1)
                    {
                        this->message = "Failed to close file";
                        throw 500;
                    }
                }
            }
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
