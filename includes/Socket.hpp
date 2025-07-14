#ifndef SOCKET_HPPP
#define SOCKET_HPPP

#include <iostream> // For cout
#include <cstring> // for strerror
#include <sys/socket.h> // For socket functions
#include <unistd.h> // For close and read (fd)
#include <arpa/inet.h> // For sockaddr_in struct
#include <cerrno> // for errno, check if c++11
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <vector>
#include <fcntl.h>
#include "Listen.hpp"
#include "ServerConf.hpp"
#define MAXCONNECTIONS 100
// creat one socket 
        
// Factory designe
class Socket{

    private:
        int         _fd;
        std::string _host;
        std::string _port;

        Socket(const std::string&, const std::string&);
        void socket_start();
        void socket_fcntl();
        void socket_bind();
        void socket_listen();

    public:
        static int StartSocket(const std::string&, const std::string&);
        // add canonical form
        // ~Socket();

};

 #endif
