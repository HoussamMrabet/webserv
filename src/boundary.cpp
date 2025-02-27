/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Boundary.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 20:01:43 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/27 02:53:17 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "boundary.hpp"

Boundary::Boundary() {}

Boundary::~Boundary()
{
    if (file.is_open())
    {
        file.close();
    }
}

void Boundary::setFileName(const std::string &fileName)
{
    this->fileName = fileName;
    this->file = std::ofstream("./" + fileName, std::ios::out | std::ios::binary | std::ios::trunc);
}

void Boundary::setHeaders(const std::map<std::string, std::string> &headers)
{
    this->headers = headers;
}

void Boundary::setContent(const std::string &content)
{
    this->content = content;
}

std::string Boundary::getFileName() const
{
    return fileName;
}

std::map<std::string, std::string> Boundary::getHeaders() const
{
    return headers;
}

std::string Boundary::getContent() const
{
    return content;
}

void Boundary::writeToFile(const std::string &content)
{
    if (!this->file.is_open())
    {
        std::cerr << "File is not open!" << std::endl;
        return;
    }
    
    this->file.write(content.c_str(), content.size());
    this->file.flush();
    
    if (this->file.fail())
    {
        std::cerr << "Failed to write to the file!" << std::endl;
    }
}