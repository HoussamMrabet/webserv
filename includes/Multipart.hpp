/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 20:01:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/15 08:23:27 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <fcntl.h>
#include <unistd.h>

typedef enum e_multipartStep
{
    MULTIPART_HEADERS,
    MULTIPART_BODY,
    MULTIPART_DONE
} t_multipartStep;


class Multipart
{
    private:
        std::string fileName;
        int file;
        std::map<std::string, std::string> headers;
        std::string contentType;
        t_multipartStep  currentStep;

        
    public:
        Multipart();
        ~Multipart();
        
        void setFileName(const std::string &fileName);
        void setHeaders(const std::map<std::string, std::string> &headers);
        void setContentType(const std::string &contentType);
        void setCurrentStep(t_multipartStep step);
        
        std::string getFileName() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getContentType() const;
        t_multipartStep getCurrentStep() const;
        void writeToFile(const std::string &content);
        void closeFile();
        void unlinkFile();

        // Inside class Multipart
        std::string getData() const;       // Returns the content of the file

};
