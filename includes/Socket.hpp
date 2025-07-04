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
#define INCOMING_CONNECTIONS 10

// this class creats one socket 
        
    class Socket{

    private:
        Listen _listen; // to remove later!
        // std::string _host;
        // std::string _port;
        int _fd;
        struct sockaddr_in _address; // can be removed
    
    public:
        // add canonical form?
        Socket(Listen&);
        // Socket();
        void socket_init(); // useless!!
        void socket_fcntl();
        void socket_start();
        void socket_bind();
        void socket_listen();
        int getFd() const;
        Listen getListen() const;
        ~Socket();
 };
 #endif
 