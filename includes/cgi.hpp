/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.hpp                                            :+:      :+:    :+:   */
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
#include <sys/wait.h> // pipe - fork - dup2 - close - execve - write - read - waitpid - STDOUT_FILENO
#include <unistd.h> // close - read - write - execve - STDIN_FILENO - fork - pipe
#include <cstdio> // perror
#include <cstdlib> // exit
#include <fcntl.h>  // fcntl, O_NONBLOCK
#include <sys/time.h> // gettimeofday
#include <sys/stat.h> // stat
#include <vector>
#include <map>
// #include <unistd.h> // for close, read, write
// #include <cerrno>   // for errno

#define SERVERERROR "HTTP/1.1 500 Internal Server Error\r\n" \
                    "Content-Type: text/plain\r\n" \
                    "Content-Length: 19\r\n\r\n" \
                    "Invalid CGI script."

class CGI{ // should class name be camel-case??
    
    private:
        // env variables:
        ServerConf _server;
        std::string _scriptName;        // filesystem path to script
        std::string _scriptFileName;        // full path to script
        std::string _location;
        std::string _execPath;
        std::string _root;
        std::string _requestMethod;     // GET, POST, etc.
        std::string _queryString;       // stuff after '?'
        std::string _body;              // POST body (if any)
        std::string _contentLenght;     // _body.size()
        std::string _contentType;       // from the headers map (should be!!)
        std::string _remoteAddr;        // remote client IP
        std::string _output;
        // std::string _errormsg;
        int _fd_in, _fd_out;
        bool _execDone;
        bool _readDone;
        /* To fix the path to file, join with root
        // if there was no root add a default path!
        std::string _location;
        std::string _root;
        */
        std::map<std::string, std::string> _headers; // HTTP headers
        std::vector<std::string> _envs; // environment variables (string)
        std::vector<char*> _envc;       // environment for execve (should be char*)
        std::string _cgiFileName;
        std::string _cgiExecPath;

        // int generateCgiFile();
        // std::string getCGIPath();
        // void getRoot();
        // void generateCgiFile();
        // CGI();
        void importData(Request&); // remove const
        // void setQueryString();
        // std::string setPath(); // remove const
        void setContentLenght();
        void set_HTTP_Header();
        void printEnvironment(); // to remove later
        // std::string parseOutput(std::string &);
        // std::string runCGI();
        bool setToNonBlocking();
        bool validPath();
        bool validExec();
        
    public:
        CGI();
        ~CGI();
        std::string executeCGI(Request&, ServerConf&); // remove const in order to be able to change status code of request 
        std::string readOutput();
        int getFd();
        bool readDone();
        bool execDone();
};
