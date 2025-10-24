#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Connection.hpp"
#include "ServerConf.hpp"
#include "Socket.hpp"
#include "Utils.hpp"
#include <poll.h>
#include <iostream>
#include <vector>
#include <map>
#define TIMEOUT 60

class WebServer {

private:
    ServerConf& config;
    std::vector<struct pollfd> fds;
    std::map<int, Connection*> conns;
    
    void addPoll(int fd, short events);
    void removePoll(int fd);
    void modifyPoll(int fd, short events);
    void closeConn(int fd);
    void cleanup();
    void acceptClient(int listen_fd);
    bool readRequest(Connection* c);    
    bool processRequest(Connection* c);
    bool startCGI(Connection* c);
    bool readCGI(Connection* cgi);
    void finishCGI(Connection* cgi);
    bool writeResponse(Connection* c);
    int startSocket(const std::string& host, const std::string& port);
    void execCGI(const Request& req, ServerConf& config);

public:
    WebServer(ServerConf& cfg);
    ~WebServer();
    static bool running;
    bool start();
    void run();
};

#endif