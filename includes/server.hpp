/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:07 by hmrabet           #+#    #+#             */
/*   Updated: 2025/01/19 15:16:23 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream> // For cout
#include <cstring> // for strerror
#include <sys/socket.h> // For socket functions
#include <unistd.h> // For close and read (fd)
#include <arpa/inet.h> // For sockaddr_in struct
#include <cerrno> // for errno, check if c++11
#include <cstdlib> // For exit() and EXIT_FAILURE
// #include <netinet/in.h> // For sockaddr_in struct
// #include <sstream> // For ostringstream
// #include <string>
// #include <stdio.h>

//-----------------------------------------------------------------
// struct sockaddr_in{ // defined in <arpa/inet.h>
//     short            sin_family;   // e.g. AF_INET
//     unsigned short   sin_port;     // e.g. htons(8080)
//     struct in_addr   sin_addr;     // see struct in_addr, below
//     char             sin_zero[8];  // zero this if you want to
// };
// struct in_addr{
//     unsigned long s_addr;
// };
//-----------------------------------------------------------------

class Server{
    public:
        Server(std::string ipAddress, int port);
        ~Server();
    private:
        std::string _ipAddress;
        int _port;
        int _socket;
        int _newSocket;
        long _incomingMessage;
        struct sockaddr_in _socketAddress;
        unsigned int _socketAddressSize;
        std::string _serverMessage;
    
        int startServer();
        void closeServer();
        void startListen();
        void acceptConnection(int &newSocket);
        void readMessage();
};
