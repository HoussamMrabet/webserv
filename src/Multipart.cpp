/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 20:01:43 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/15 03:40:17 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Multipart.hpp"

Multipart::Multipart() : currentStep(MULTIPART_HEADERS) {}

Multipart::~Multipart()
{
    if (file.is_open())
    {
        file.close();
    }
}

void Multipart::setFileName(const std::string &fileName)
{
    this->fileName = fileName;
    this->file = std::ofstream("./" + fileName, std::ios::out | std::ios::binary | std::ios::trunc);
}

void Multipart::setHeaders(const std::map<std::string, std::string> &headers)
{
    this->headers = headers;
}

void Multipart::setContentType(const std::string &contentType)
{
    this->contentType = contentType;
}

void Multipart::setContent(const std::string &content)
{
    this->content = content;
}

void Multipart::setCurrentStep(t_multipartStep step)
{
    this->currentStep = step;
}

std::string Multipart::getFileName() const
{
    return fileName;
}

std::map<std::string, std::string> Multipart::getHeaders() const
{
    return headers;
}

std::string Multipart::getContentType() const
{
    return contentType;
}

std::string Multipart::getContent() const
{
    return content;
}

t_multipartStep Multipart::getCurrentStep() const
{
    return currentStep;
}

void Multipart::writeToFile(const std::string &content)
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

void Multipart::closeFile()
{
    if (this->file.is_open())
        this->file.close();
}