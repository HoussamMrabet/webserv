#ifndef SOCKET_HPPP
#define SOCKET_HPPP

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <vector>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "ServerConf.hpp"
#define MAXCONNECTIONS 100
        
class Socket{

    private:
        int         _fd;
        std::string _host;
        std::string _port;
        struct addrinfo _info, *_ip;

        Socket(const std::string&, const std::string&);
        void socket_start();
        void socket_fcntl();
        void socket_bind();
        void socket_listen();

    public:
        static int StartSocket(const std::string&, const std::string&);
        ~Socket();

};

 #endif
