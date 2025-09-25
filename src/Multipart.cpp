/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 20:01:43 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/01 11:49:50 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Multipart.hpp"

Multipart::Multipart() : currentStep(MULTIPART_HEADERS) {}

Multipart::~Multipart()
{
    if (this->file != -1)
    {
        close(this->file);
        this->file = -1;
    }
}

void Multipart::setFileName(const std::string &fileName)
{
    this->fileName = fileName;

    this->file = open(fileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (this->file == -1)
    {
        throw "Failed to open file";
    }
}

void Multipart::setHeaders(const std::map<std::string, std::string> &headers)
{
    this->headers = headers;
}

void Multipart::setContentType(const std::string &contentType)
{
    this->contentType = contentType;
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

t_multipartStep Multipart::getCurrentStep() const
{
    return currentStep;
}

void Multipart::writeToFile(const std::string &content)
{
    if (this->file == -1)
        throw "File is not open!";

    ssize_t written = ::write(this->file, content.c_str(), content.size());
    if (written == -1)
        throw "Failed to write to the file!";
}

void Multipart::closeFile()
{
    if (this->file != -1)
        close(this->file);
}

void Multipart::unlinkFile()
{
    if (this->file != -1)
    {
        close(this->file);
        this->file = -1;
    }
    if (!this->fileName.empty())
    {
        remove(this->fileName.c_str());
        this->fileName.clear();
    }
}