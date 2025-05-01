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
// #include "ServerConf.hpp"

// creat one socket 
        
    class Socket{
        public:
        // add canonical form
        Socket(std::string host, std::string port);
        int getFd() const;
        ~Socket();
    private:
        std::string _host;
        std::string _port;
        int _fd;
        // std::vector<std::string> _serverNames;
        struct sockaddr_in _address; // can be removed

 };
 #endif
 