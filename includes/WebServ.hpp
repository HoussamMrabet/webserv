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
        std::map<int, Connection> _connections;
        std::map<int, int> _cgifds;
        std::map<int, std::string> _fdType;
        WebServ(ServerConf&);
        void pollLoop();
        bool acceptConnection(int);
        bool checkStatus(int);
        void cleanUp();
        void addPollFd(int, short, const std::string&);

    public:
        ~WebServ();
        static bool _runServer;
        static bool startServer(ServerConf&);
};

#endif