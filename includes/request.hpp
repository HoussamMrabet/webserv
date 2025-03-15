/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:29 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/15 07:13:51 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include "Multipart.hpp"

#define CHUNKED true

typedef enum e_method
{
    UNDEFINED,
    GET,
    POST,
    DELETE,
} t_method;

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
        std::string message;
        t_step      currentStep;
        t_method    method;
        std::string reqLine;
        std::string uri;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::string body;
        std::ofstream file;
        bool isChunked;
        bool isMultipart;
        bool isContentLength;
        std::string boundaryKey;
        size_t contentLength;
        std::string requestData;
        std::string headersData;
        size_t currentContentLength;
        std::string fileName;
        std::string fullBody;
        std::vector<Multipart *> multipartData;
        size_t chunkSize;
        std::string chunkData;
        bool   inChunk;;
        void parseRequestLine();
        void parseHeaders();
        void parseBody();
        void setBodyInformations();
        void parseMultipart(bool isChunked = false);
        void parseMultipartHeaders(const std::string &multipartHeaders);

    public:
        Request();
        ~Request();
        
        t_method getMethod() const;
        std::string getUri() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::string getHeader(const std::string &key) const;
        t_step getCurrentStep() const;
        int getStatusCode() const;

        void parseRequest(const std::string& rawRequest = "");
        void printRequest();
};

void handleUriSpecialCharacters(std::string &uri);
std::string generateRandomFileName(const std::string &prefix = "");
