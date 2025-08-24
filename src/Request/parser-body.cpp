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
        if (this->body.empty())
            parseMultipart(CHUNKED);
        if (inChunk)
        {
            if (this->body.size() < this->chunkSize + 2)
                return ;
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
                        this->file.write(this->body.substr(0, this->chunkSize).c_str(), this->chunkSize);
                        this->file.flush();
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
                this->body.clear();
                throw 200;
            }
        }
    }
    else if (this->isMultipart && !this->isCGI())
    {
        parseMultipart();
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
                this->file.write(this->body.c_str(), this->body.size());
                this->file.flush();
                this->file.close();
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
