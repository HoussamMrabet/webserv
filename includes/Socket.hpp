// #pragma once

// #include <string>
// #include <netinet/in.h>  // sockaddr_in
// #include <stdexcept>

// // RAII wrapper for a non-blocking listen socket
// class Socket {
// public:
//     Socket(const std::string& host, int port, int backlog = 128);
//     ~Socket();

//     int getFd() const;
//     void bindAndListen();
//     void setNonBlocking();
//     void close();

//     // Disable copy
//     Socket(const Socket&) = delete;
//     Socket& operator=(const Socket&) = delete;

//     // Allow move
//     Socket(Socket&& other);
//     Socket& operator=(Socket&& other);

// private:
//     int _fd;
//     sockaddr_in _addr;
//     bool _is_listening;

//     void initSocket();
// };
