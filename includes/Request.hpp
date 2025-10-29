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
#include "ServerConf.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#define G "\033[1;32m"
#define R "\033[1;31m"
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

typedef struct s_user
{
    std::string username;
    std::string password;
    std::string email;
    std::string fullName;
    std::string avatar;
    std::string job;
} t_user;

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
        std::string uriIndexe;
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
        std::string fullPath;
        size_t bodySizeLimit;
        std::string root;
        std::string uploadDir;
        std::vector<Multipart *> multipartData;
        size_t chunkSize;
        std::string chunkData;
        bool inChunk;
        int cgiFdRead;
        int cgiFdWrite;
        ServerConf server;
        void parseRequestLine();
        void parseHeaders();
        void handleThemeCookie();
        void handleSession();
        void parseBody();
        void setBodyInformations();
        void processResponseErrors();
        void parseMultipart(bool isChunked = false);
        void parseMultipartHeaders(const std::string &multipartHeaders);
        std::string extractQueryParam(const std::string& queryString, const std::string& paramName);
        bool lastChunk;
    public:
        static std::string theme;
        static std::vector<t_user> users;
        static t_user loggedInUser;
        static bool loggedIn;

        Request();
        ~Request();
        Request(const Request&);
        
        t_method getMethod() const;
        std::string getStrMethod() const;
        std::string getUri() const;
        std::string getUriQueries() const;
        std::string getUriFileName() const;
        std::string getLocation() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::string getFullPath() const;
        std::string getFullUri() const;
        std::string getHeader(const std::string &key) const;
        std::string getHost() const;
        std::string getCgiType() const;
        int getStatusCode() const;
        void setStatusCode(int);
        int getCgiFdRead() const;
        std::string getRoot() const;

        bool isDone() const;
        bool isCGI() const;
        void CGIError();
        void parseRequest(const std::string& rawRequest = "");
        void printRequest();
        std::string getMessage() const;
        void processRequest();
};

void handleUriSpecialCharacters(std::string &uri);
std::string generateRandomFileName(const std::string &prefix = "");
void checkMediaType(const std::string &contentType);
