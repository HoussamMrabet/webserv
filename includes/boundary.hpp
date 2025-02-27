/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Boundary.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 20:01:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/26 22:23:41 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <map>

class Boundary
{
    private:
        std::string fileName;
        std::ofstream file;
        std::map<std::string, std::string> headers;
        std::string content;
        
    public:
        Boundary();
        ~Boundary();
        
        void setFileName(const std::string &fileName);
        void setHeaders(const std::map<std::string, std::string> &headers);
        void setContent(const std::string &content);
        
        std::string getFileName() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getContent() const;
        void writeToFile(const std::string &content);
};
