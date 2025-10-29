/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:32 by hmrabet           #+#    #+#             */
/*   Updated: 2025/01/19 15:16:34 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ServerConf.hpp"
#include "Request.hpp"
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <vector>
#include <map>

#define SERVERERROR "HTTP/1.1 500 Internal Server Error\r\n" \
                    "Content-Type: text/plain\r\n" \
                    "Content-Length: 19\r\n\r\n" \
                    "Invalid CGI script."

class CGI{
    
    private:
        ServerConf _server;
        std::string _scriptName;
        std::string _scriptFileName;
        std::string _location;
        std::string _execPath;
        std::string _root;
        std::string _requestMethod;
        std::string _queryString; 
        std::string _body;
        std::string _contentLenght;
        std::string _contentType;
        std::string _remoteAddr;
        std::string _output;
        int _fd_in, _fd_out;
        bool _execDone;
        bool _readDone;
        std::map<std::string, std::string> _headers;
        std::vector<std::string> _envs;
        std::vector<char*> _envc;
        std::string _cgiFileName;
        std::string _cgiExecPath;

        void importData(Request&);
        void setContentLenght();
        void set_HTTP_Header();
        void printEnvironment();
        bool setToNonBlocking();
        bool validPath();
        bool validExec();
        
    public:
        CGI();
        ~CGI();
        std::string executeCGI(Request&, ServerConf&);
        std::string readOutput();
        int getFd();
        bool readDone();
        bool execDone();
};
