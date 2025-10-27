#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Connection.hpp"
#include "ServerConf.hpp"
#include "Socket.hpp"    
#include <poll.h>
// #include <csignal>
#include <iostream>
#include <vector>
#include <map>
#define TIMEOUT 60

class WebServ{ // Factory design

    private:
        ServerConf& _server;
        std::map<int, std::pair<std::string, std::string> > _sockets;
        std::vector<struct pollfd> _pollfds;
        std::map<int, Connection> _connections; // can use just vector?
        std::map<int, int> _cgifds; // can use just vector?
        std::map<int, std::string> _fdType; // to check if fd is a listen, client connection, or a cgi pipe
        // bool _cleanRead;
        // WebServ();
        WebServ(ServerConf&);
        // void startSocket(std::string, std::string);
        void pollLoop();
    //     bool newConnection(int);
    //     bool isConnection(int);
    //     bool isListening(int);
        bool acceptConnection(int);
        bool checkStatus(int);
        // void checkTimeout();
        void cleanUp();
        void addPollFd(int, short, const std::string&);
    //     void removePollFd(int);

    public:
        ~WebServ();
        static bool _runServer;
        static bool startServer(ServerConf&);
        // static void signalHandler(int);
};

#endif