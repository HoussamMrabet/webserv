#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <poll.h>
#include "Socket.hpp"    
// #include "Listen.hpp"
#include "Connection.hpp"
#include "Request.hpp"
#include "Response.hpp"
// #include "ServerConf.hpp"

// class ServerConf;
// class Request;
// class Response;

extern std::vector<ServerConf> globalServer;

struct Connection {
    enum Type { LISTEN, CLIENT, CGI };
    enum State { READING, PROCESSING, WRITING, DONE };
    
    int fd;
    Type type;
    State state;
    
    // Client data
    Request* req;
    Response* resp;
    std::string read_buf;
    std::string write_buf;
    size_t write_pos;
    
    // CGI data  
    pid_t pid;
    int parent_fd;
    time_t activity;
    
    Connection(Type);
    ~Connection();
    void cleanup(); 
    void touch();
    bool expired() const;
};

class WebServer{ // Factory design
    private:
        // // std::vector<int> _serverfds; // can be removed?
        // ServerConf _server;
        // std::vector<std::pair<std::string, std::string> > _listens;
        // std::vector<struct pollfd> _pollfds;
        // std::map<int, Connection*> _connections; // can use just vector?
        // std::map<int, Connection*> _cgis; // can use just vector?
        // std::map<int, std::string> _fdType; // to check if fd is a listen, client connection, or a cgi pipe
        // bool _cleanRead;
        // bool _cleanWrite;

        ServerConf& config;
        std::vector<struct pollfd> fds;
        std::map<int, Connection*> conns;


    //     WebServ();
    //     WebServ(ServerConf&);
    //     // void startSocket(std::string, std::string);
    //     void pollLoop();
    // //     bool newConnection(int);
    // //     bool isConnection(int);
    // //     bool isListening(int);
    //     bool acceptConnection(int);
    //     void checkTimeout();
    //     void addPollFd(int, short, const std::string&);
    // //     void removePollFd(int);

    // public:
    //     ~WebServ();
    //     static bool startServer(ServerConf&);


    public:
        WebServer(ServerConf& cfg);
        ~WebServer();
        bool start();
        void run();

    private:
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
        int createListenSocket(const std::string& host, const std::string& port);
        void execCGI(const Request& req, ServerConf& config);
};
