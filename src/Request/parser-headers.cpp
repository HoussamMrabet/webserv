/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser-headers.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 07:46:04 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/17 00:09:45 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::string Request::theme = "light";

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

    if (this->headers.size() > 200)
    {
        this->message = "Too Many Headers";
        throw 431;
    }
}

std::string extractThemeFromCookie(const std::string& cookieString) {
    // Look for "theme=" in the cookie string
    size_t themePos = cookieString.find("theme=");
    if (themePos != std::string::npos) {
        size_t valueStart = themePos + 6; // Length of "theme="
        size_t valueEnd = cookieString.find(";", valueStart);
        if (valueEnd == std::string::npos) {
            valueEnd = cookieString.length();
        }
        std::string themeValue = cookieString.substr(valueStart, valueEnd - valueStart);
        
        // Remove any trailing whitespace or carriage return
        while (!themeValue.empty() && (themeValue.back() == ' ' || themeValue.back() == '\t' || themeValue.back() == '\r')) {
            themeValue.pop_back();
        }
        
        return themeValue;
    }
    return "";
}

void Request::handleThemeCookie()
{
    // Check if URI is /profile or /profile/login.html
    if (uri == "/profile" || uri == "/profile/login.html") {
        
        // Look for existing Cookie header
        bool cookieFound = false;
        std::string cookieValue;
        
        // Search through headers for Cookie header
        for (std::map<std::string, std::string>::iterator it = this->headers.begin(); 
             it != this->headers.end(); ++it) {
            if (it->first == "cookie") {
                cookieFound = true;
                cookieValue = it->second;
                
                // Extract theme value from cookie string
                std::string themeValue = extractThemeFromCookie(cookieValue);
                if (!themeValue.empty()) {
                    Request::theme = themeValue;
                }
                break;
            }
        }
        
        // If no Cookie header found, add one with current theme
        if (!cookieFound) {
            std::string cookieHeader = "theme=" + Request::theme + "; Path=/; Max-Age=2592000";
            headers["Set-Cookie"] = cookieHeader;
        }
    }
}
