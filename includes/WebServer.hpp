#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <poll.h>
#include "Socket.hpp"    
#include "Listen.hpp"  
#include "ServerBlock.hpp"
#include "Connection.hpp"

class WebServ{ // Factory design
    private:
        // std::vector<int> _serverfds; // can be removed?
        std::vector<struct pollfd> _pollfds;
        std::map<Listen, ServerBlock > _serverBlocks;
        std::map<int,  Connection> _connections; // can use just vector?
        std::map<int, std::string> _fdType; // to check if fd is a listen, client connection, or a cgi pipe

        WebServ();
        WebServ(std::map<Listen, ServerBlock >&);
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
        static bool startServer(std::map<Listen, ServerBlock >&);
};