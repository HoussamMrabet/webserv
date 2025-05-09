/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-body.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmasnaou <cmasnaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 07:48:15 by hmrabet           #+#    #+#             */
/*   Updated: 2025/05/09 09:20:11 by cmasnaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request.hpp"

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
