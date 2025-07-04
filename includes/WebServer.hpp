// #pragma once

// #include <vector>
// #include <poll.h>
// #include "ServerConf.hpp"
// #include "Socket.hpp"
// #include "Connection.hpp"

// // Main server loop managing multiple listening sockets + active connections
// class WebServer {
// public:
//     explicit WebServer(const std::vector<ServerConfig>& configs);
//     ~WebServer();

//     void run();

// private:
//     std::vector<Socket> _listeners;
//     std::vector<pollfd> _fds;
//     std::vector<Connection> _connections;
//     std::vector<ServerConfig> _configs;

//     void setupListeners();
//     void updatePollFds();
//     void acceptNewConnections();
//     void handleReadyFds();
//     void closeAndRemove(Connection& conn);

//     ssize_t findListenerConfig(int fd) const;

//     static const int POLL_TIMEOUT = 1000; // 1 second
// };
