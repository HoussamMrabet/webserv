#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <poll.h>
#include <signal.h>
#include "Socket.hpp"    
// #include "Listen.hpp"
#include "Connection.hpp"
// #include "ServerConf.hpp"


// extern std::vector<ServerConf> globalServer;

class WebServ{ // Factory design
    private:
        // std::vector<int> _serverfds; // can be removed?
        ServerConf _server;
        // std::vector<std::pair<std::string, std::string> > _listens;
        std::map<int, std::pair<std::string, std::string> > _listenFds;
        std::vector<struct pollfd> _pollfds;
        std::map<int, Connection> _connections; // can use just vector?
        std::map<int, int> _cgifds; // can use just vector?
        std::map<int, std::string> _fdType; // to check if fd is a listen, client connection, or a cgi pipe
        bool _cleanRead;
        WebServ();
        WebServ(ServerConf&);
        // void startSocket(std::string, std::string);
        void pollLoop();
    //     bool newConnection(int);
    //     bool isConnection(int);
    //     bool isListening(int);
        bool acceptConnection(int);
        void checkTimeout();
        void addPollFd(int, short, const std::string&);
    //     void removePollFd(int);

    public:
        ~WebServ();
        static bool _runServer;
        static bool startServer(ServerConf&);
        static void signalHandler(int);
};