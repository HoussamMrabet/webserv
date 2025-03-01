/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:29 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/01 06:39:44 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include "boundary.hpp"

typedef enum e_method
{
    UNDEFINED,
    GET,
    POST,
    DELETE,
    INVALID,
} t_method;

typedef enum e_reqState
{
    VALID,
    INVALID_REQ_LINE,
    INVALID_METHOD,
    HOST_MISSING,
    INVALID_HEADERS,
} t_reqState;

typedef enum e_step
{
    REQ_LINE,
    HEADERS,
    BODY,
    DONE
} t_step;

class Request
{
    private:
        int         statusCode;
        t_step      currentStep;
        t_reqState  state;
        t_method    method;
        std::string reqLine;
        std::string uri;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::string body;
        bool isChunked;
        bool isBoundary;
        bool isContentLength;
        std::string boundaryKey;
        size_t contentLength;
        std::string requestData;
        std::string headersData;
        size_t currentContentLength;
        std::string fileName;
        std::ofstream file;
        std::string fullBody;
        std::vector<Boundary *> boundaries;
        void parseRequestLine();
        void parseHeaders();
        void parseBody();
        void setBodyInformations();

    public:
        Request();
        ~Request();

        void setReqLine(const std::string &reqLine);
        void setMethod(t_method &method);
        void setUri(const std::string &uri);
        void setHttpVersion(const std::string &httpVersion);
        void setHeaders(const std::map<std::string, std::string> &headers);
        void setBody(const std::string &body);
        
        t_method getMethod() const;
        std::string getReqLine() const;
        std::string getUri() const;
        std::string getHttpVersion() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::string getHeader(const std::string &key) const;
        t_reqState getState() const;
        t_step getCurrentStep() const;
        int getStatusCode() const;

        void addHeader(const std::string &key, const std::string &value);
        void parseRequest(const std::string& rawRequest);
        void printRequest();
};
