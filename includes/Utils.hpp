#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <map>
#include <string>
#include <poll.h>
#include <unistd.h>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctime>
#include <cstdlib>
#include <iostream>

#include "Request.hpp"
#include "Response.hpp"
#include "ServerConf.hpp"

// class ServerConf;
// class Request;
// class Response;

struct Connection
{
    enum Type 
    { 
        LISTEN, 
        CLIENT, 
        CGI 
    };
    
    enum State
    { 
        READING, 
        PROCESSING, 
        WRITING, 
        DONE 
    };
    
    Type type;
    State state;
    int fd;
    time_t activity;
    
    // Client data
    std::string read_buf;
    std::string write_buf;
    size_t write_pos;
    Request* req;
    Response* resp;
    
    // CGI data  
    int parent_fd;
    pid_t pid;
    
    Connection(Type t) : type(t), state(READING), fd(-1), 
        activity(time(NULL)), write_pos(0), req(NULL), resp(NULL),
        parent_fd(-1), pid(-1) {}
    
    ~Connection() { 
        cleanup(); 
    }
    
    void cleanup() {
        delete req; 
        delete resp;
        req = NULL;
        resp = NULL;
        if (pid > 0) { 
            kill(pid, SIGTERM); 
            waitpid(pid, NULL, WNOHANG); 
        }
    }
    
    void touch() { 
        activity = time(NULL); 
    }
    
    bool expired() const { 
        return time(NULL) - activity > 30; 
    }
};

#endif