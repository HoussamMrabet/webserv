#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <poll.h>
#include "Socket.hpp"    
// #include "Listen.hpp"
#include "Connection.hpp"
#include "response2.hpp"
#define TIMEOUT 60
// #include "ServerConf.hpp"

struct fdInfo{
    enum Type {LISTEN, CLIENT, CGI};
    enum State {READING, PROCESSING, WRITING, DONE};
    
    int         fd;
    Type        type;
    State       state;
    Request*    req;
    Response2*   resp;
    
    // Client data
    std::string read_buf;
    std::string write_buf;
    size_t      write_pos;
    time_t      activity;
    
    // CGI data  
    int         parent_fd;
    pid_t       pid;
    
    fdInfo(Type);
    ~fdInfo();
    void timer();    
    bool timedout();
};

extern std::vector<ServerConf> globalServer;

class WebServ{ // Factory design
    private:
        /*********************/
        std::map<int /*fd*/, fdInfo*> conns;
        std::vector<struct pollfd> fds;
        void addPoll(int fd, short events);
        void cleanup();
        void closeConn(int fd);
        void removePoll(int fd);
        void modifyPoll(int fd, short events);
        void acceptClient(int listen_fd);
        bool readRequest(fdInfo* c);
        bool processRequest(fdInfo* c);
        bool startCGI(fdInfo* c);
        bool readCGI(fdInfo* c);
        void finishCGI(fdInfo* cgi);
        bool writeResponse(fdInfo* c);
        void execCGI(const Request& req, ServerConf& config);

        /*********************/


        // std::vector<int> _serverfds; // can be removed?
        ServerConf& _server;
        std::vector<std::pair<std::string, std::string> > _listens;
        // std::vector<struct pollfd> _pollfds;
        // std::map<int, Connection*> _connections; // can use just vector?
        // std::map<int, std::string> _fdType; // to check if fd is a listen, client connection, or a cgi pipe
        // bool _cleanRead;
        // // WebServ();
        WebServ(ServerConf&);
        // void startSocket(std::string, std::string);
        void pollLoop();
    //     bool newConnection(int);
    //     bool isConnection(int);
    //     bool isListening(int);
        // bool acceptConnection(int);
        // void checkTimeout();
        // void addPollFd(int, short, const std::string&);
    //     void removePollFd(int);

    public:
        ~WebServ();
        static bool startServer(ServerConf&);
};