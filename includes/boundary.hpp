/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   boundary.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 20:01:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/05 14:29:09 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <map>

typedef enum e_boundaryStep
{
    BOUNDARY_HEADERS,
    BOUNDARY_BODY,
    BOUNDARY_DONE
} t_boundaryStep;


class Boundary
{
    private:
        std::string fileName;
        std::ofstream file;
        std::map<std::string, std::string> headers;
        std::string content;
        t_boundaryStep  currentStep;
        
    public:
        Boundary();
        ~Boundary();
        
        void setFileName(const std::string &fileName);
        void setHeaders(const std::map<std::string, std::string> &headers);
        void setContent(const std::string &content);
        void setCurrentStep(t_boundaryStep step);
        
        std::string getFileName() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getContent() const;
        t_boundaryStep getCurrentStep() const;
        void writeToFile(const std::string &content);
};
