#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include <iostream>
#include <sys/socket.h> // accept 
#include <fcntl.h> // fcntl
#include <unistd.h>   // read, write, close
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConf.hpp"
#include "cgi.hpp"

class Connection{
private:
    int _fd;
    time_t _time;
    std::string _buffer;
    Request* _request;
    // Response _response;
    std::string _response;
    static ServerConf _server;
    bool _done;

public:
    Connection(int, ServerConf&);
    Connection(const Connection&);
    int getFd() const;
    ServerConf getServer();
    time_t getTime() const;
    bool readRequest();
    bool isDone();
    bool writeResponse();
    void printRequest(); // to remove
    void setNonBlocking();
    std::string getRequestMethod();
    void sendGetResponse();
    // bool cgiResponse(int (&fd_in)[], int (&fd_out)[]);
};

#endif
