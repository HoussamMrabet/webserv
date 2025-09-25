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
        catch (const char *e)
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

std::string extractThemeFromCookie(const std::string &cookieString)
{
    size_t themePos = cookieString.find("theme=");
    if (themePos != std::string::npos)
    {
        size_t valueStart = themePos + 6;
        size_t valueEnd = cookieString.find(";", valueStart);
        if (valueEnd == std::string::npos)
        {
            valueEnd = cookieString.length();
        }
        std::string themeValue = cookieString.substr(valueStart, valueEnd - valueStart);

        while (!themeValue.empty() && (themeValue.back() == ' ' || themeValue.back() == '\t' || themeValue.back() == '\r'))
        {
            themeValue.pop_back();
        }

        return themeValue;
    }
    return "";
}

void Request::handleThemeCookie()
{
    if (uri == "/profile" || uri == "/profile/login.html" || uri == "/profile/profile.html")
    {

        bool cookieFound = false;
        std::string cookieValue;

        for (std::map<std::string, std::string>::iterator it = this->headers.begin();
             it != this->headers.end(); ++it)
        {
            if (it->first == "cookie")
            {
                cookieFound = true;
                cookieValue = it->second;

                std::string themeValue = extractThemeFromCookie(cookieValue);
                if (!themeValue.empty())
                {
                    Request::theme = themeValue;
                }
                break;
            }
        }

        if (!cookieFound)
        {
            std::string cookieHeader = "theme=" + Request::theme + "; Path=/; Max-Age=3600";
            headers["Set-Cookie"] = cookieHeader;
        }
    }
}

void Request::handleSession()
{
    if (uri == "/profile" || uri == "/profile/login.html" || uri == "/profile/profile.html")
    {

        if (Request::loggedIn)
        {
            headers["X-User-Username"] = Request::loggedInUser.username;
            headers["X-User-Email"] = Request::loggedInUser.email;
            headers["X-User-FullName"] = Request::loggedInUser.fullName;
            headers["X-User-Avatar"] = Request::loggedInUser.avatar;
            headers["X-User-Job"] = Request::loggedInUser.job;
        }

        // Parse query parameters from URI
        std::string queryString = getUriQueries();

        if (!queryString.empty())
        {
            std::string username = extractQueryParam(queryString, "username");
            std::string password = extractQueryParam(queryString, "password");

            // Compare with users vector
            for (std::vector<t_user>::iterator it = Request::users.begin();
                 it != Request::users.end(); ++it)
            {
                if (it->username == username && it->password == password)
                {
                    // Match found - set logged in user
                    Request::loggedInUser = *it;
                    Request::loggedIn = true;
                    return;
                }
            }
            // No match found - do nothing (loggedIn remains false)
        }
    }
    if (uri == "/logout")
    {
        std::cout << "Logging out user: " << Request::loggedInUser.username << std::endl;
        Request::loggedIn = false;
        Request::loggedInUser = t_user(); // Reset to empty user
        // Send appropriate response
    }
}

// Helper function to extract query parameter value
std::string Request::extractQueryParam(const std::string &queryString, const std::string &paramName)
{
    std::string searchPattern = paramName + "=";
    size_t startPos = queryString.find(searchPattern);

    if (startPos == std::string::npos)
    {
        return "";
    }

    startPos += searchPattern.length();
    size_t endPos = queryString.find("&", startPos);

    if (endPos == std::string::npos)
    {
        endPos = queryString.length();
    }

    return queryString.substr(startPos, endPos - startPos);
}
