#pragma once

// #include <iostream>
// #include "Server.hpp"
// #include "Request.hpp"
// #include "response.hpp"
// #include "cgi.hpp"





// #include <iostream> // For cout
// #include <cstring> // for strerror
// #include <sys/socket.h> // For socket functions
// #include <unistd.h> // For close and read (fd)
// #include <arpa/inet.h> // For sockaddr_in struct
// #include <cerrno> // for errno, check if c++11
// #include <cstdlib> // For exit() and EXIT_FAILURE
// // #include <netinet/in.h> // For sockaddr_in struct
// // #include <sstream> // For ostringstream
// // #include <string>
// // #include <stdio.h>
// #include <poll.h> // for poll()
// #include <vector>

// //-----------------------------------------------------------------
// // int poll(struct pollfd *fds, nfds_t nfds, int timeout); 
// // struct pollfd {
//     //     int   fd;         /* file descriptor */
//     //     short events;     /* requested events */
//     //     short revents;    /* returned events */
//     // };
//     //-----------------------------------------------------------------
    
//     //-----------------------------------------------------------------
//     // struct sockaddr_in{ // defined in <arpa/inet.h>
//     //     short            sin_family;   // e.g. AF_INET
//     //     unsigned short   sin_port;     // e.g. htons(8080)
//     //     struct in_addr   sin_addr;     // see struct in_addr, below
//     //     char             sin_zero[8];  // zero this if you want to
//     // };
//     // struct in_addr{
//         //     unsigned long s_addr;
//         // };
//         //-----------------------------------------------------------------

#include "Socket.hpp"      
// class of sockets 
// each socket contains many servers

class WebServ{
    private:
        // std::vector<Server> servers;
        std::vector<Socket> sockets;

        // std::string _ipAddress;
        // int _port;
        // int _socket;
        // int _newSocket;
        // long _incomingMessage;
        // struct sockaddr_in _socketAddress;
        // unsigned int _socketAddressSize;
        // std::string _serverMessage;
        
        // int startServer();
        // void closeServer();
        // void startListen();
        // void acceptConnection(int &newSocket);
        // void readMessage();
        // void sendResponse();
        // void addClient();
        // struct pollfd addnew();
    
    public:
        static addSocket(Socket& s);
        // Server(std::string ipAddress, int port);
        // ~Server();
};