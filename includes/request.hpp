/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:29 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/21 06:47:54 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

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
        int contentLength;
        std::string requestData;
        std::string headersData;
        void parseRequestLine();
        void parseHeaders();
        void parseBody();

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
        int getStatusCode() const;

        void addHeader(const std::string &key, const std::string &value);
        void parseRequest(const std::string& rawRequest);
        void printRequest();
        bool    bodyDone;
};
