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

// creat one socket 
        
    class Socket{
        public:
        // add canonical form
        Socket(/*std::string ip, */int port);
        int getFd() const;
        ~Socket();
    private:
        // std::string _ip;
        int _port;
        int _fd;
        struct sockaddr_in _address;

 };
 #endif
 