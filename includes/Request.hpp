/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:29 by hmrabet           #+#    #+#             */
/*   Updated: 2025/08/24 20:55:48 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include "Multipart.hpp"
#include <fcntl.h>
#include <unistd.h>
#define G "\033[1;32m"
#define C "\033[1;36m"
#define M "\033[1;35m"
#define B "\033[0m"

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
        std::string uriQueries;
        std::string uriFileName;
        std::string location;
        std::string cgiType;
        std::string host;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::string body;
        int file;
        bool isChunked;
        bool isMultipart;
        bool isContentLength;
        std::string boundaryKey;
        size_t contentLength;
        std::string requestData;
        std::string headersData;
        size_t currentContentLength;
        std::string fileName;
        std::string createdFile;
        std::string fullBody;
        size_t bodySizeLimit;
        std::string root;
        std::string uploadDir;
        std::vector<Multipart *> multipartData;
        size_t chunkSize;
        std::string chunkData;
        bool inChunk;
        int cgiFdRead;
        int cgiFdWrite;
        void parseRequestLine();
        void parseHeaders();
        void parseBody();
        void setBodyInformations();
        void processResponseErrors();
        void parseMultipart(bool isChunked = false);
        void parseMultipartHeaders(const std::string &multipartHeaders);
        Request(const Request&);            // don't define!!!
        Request& operator=(const Request&); // don't define!!!
    public:
        Request();
        ~Request();
        
        t_method getMethod() const;
        std::string getStrMethod() const;
        std::string getUri() const; // return the uri (without queries if exists)
        std::string getUriQueries() const; // return the queries from the uri "?..."
        std::string getUriFileName() const; // return the file name from the uri
        std::string getLocation() const; // return the location matched in the config file
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::string getHeader(const std::string &key) const;
        std::string getHost() const; // return server name
        std::string getCgiType() const; // return php or py as string if there is a cgi otherwise return empty string
        int getStatusCode() const;
        int getCgiFdRead() const; // return read end of cgi pipe

        bool isDone() const;
        bool isCGI() const; // check if the request is a cgi or not
        void parseRequest(const std::string& rawRequest = "");
        void printRequest();
        std::string getMessage() const;
};

void handleUriSpecialCharacters(std::string &uri);
std::string generateRandomFileName(const std::string &prefix = "");
void checkMediaType(const std::string &contentType);
