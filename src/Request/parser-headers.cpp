/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-headers.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmasnaou <cmasnaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 07:46:04 by hmrabet           #+#    #+#             */
/*   Updated: 2025/05/09 09:19:53 by cmasnaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Request.hpp"

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

                this->headers[key] = value[value.size() - 1] == '\r' ? value.substr(0, value.size() - 1) : value;
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
    
    if (this->headers.find("content-type") != this->headers.end())
    {
        std::string contentType = this->headers["content-type"];
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
