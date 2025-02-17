/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:29 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/18 00:23:47 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <map>

class Request {
    private:
        std::string method;
        std::string uri;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::string body;

    public:
        Request();
        ~Request();

        void setMethod(const std::string &method);
        void setUri(const std::string &uri);
        void setHttpVersion(const std::string &httpVersion);
        void setHeaders(const std::map<std::string, std::string> &headers);
        void setBody(const std::string &body);
        
        std::string getMethod() const;
        std::string getUri() const;
        std::string getHttpVersion() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::string getHeader(const std::string &key) const;

        void addHeader(const std::string &key, const std::string &value);
        void parseRequest(const std::string& rawRequest);
        void printRequest() const;
    };
