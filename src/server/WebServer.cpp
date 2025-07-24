#include "WebServer.hpp"

WebServ::WebServ() {}

WebServ::WebServ(std::map<Listen, ServerBlock >& serverBlocks)
    : _serverBlocks(serverBlocks) {}

bool WebServ::startServer(std::map<Listen, ServerBlock >& serverBlocks) {
    WebServ webserv(serverBlocks);

    std::map<Listen, ServerBlock >::iterator it;
    for (it = webserv._serverBlocks.begin(); it != webserv._serverBlocks.end(); ++it) {
        int server_fd = Socket::StartSocket(it->first.getHost(), it->first.getPort());
        webserv.addPollFd(server_fd, POLLIN, "listen");
        std::cout << "Listening on " << it->first.getHost() << ":" << it->first.getPort() << "..." << std::endl;
    }
    webserv.pollLoop();
    return true;
}

void WebServ::addPollFd(int fd, short event, const std::string& type) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = event;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
    _fdType[fd] = type;
}

// void WebServ::pollLoop() {
//     while (true) {
//         int n = poll(&_pollfds[0], _pollfds.size(), 1000); // C++98: use &_pollfds[0] for vector data
//         if (n == -1) {
//             perror("Poll failed");
//             break;
//         }
//         checkTimeout();
//         if (n == 0)
//             continue;
//         for (size_t i = 0; i < _pollfds.size(); ++i) {
//             int fd = _pollfds[i].fd;
//             short revents = _pollfds[i].revents;
//             if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
//                 close(fd);
//                 _connections.erase(fd);
//                 _pollfds.erase(_pollfds.begin() + i);
//                 --i;
//                 continue;
//             }
//             if (revents & POLLIN) {
//                 if (_fdType[fd] == "listen") {
//                     if (!acceptConnection(fd))
//                         continue;
//                 }
//                 else if (_fdType[fd] == "connection") {
//                     std::map<int, Connection>::iterator it = _connections.find(fd);
//                     if (it == _connections.end())
//                         continue;
//                     if (!it->second.readRequest()) {
//                         close(fd);
//                         _connections.erase(it);
//                         _pollfds.erase(_pollfds.begin() + i);
//                         --i;
//                         continue;
//                     }
//                     it->second.writeResponse();
//                 }
//             }
//         }
//     }
// }


void WebServ::pollLoop() {
    while (true) {
        int n = poll(&_pollfds[0], _pollfds.size(), 1000);
        if (n == -1) {
            perror("Poll failed");
            break;
        }
        checkTimeout();
        if (n == 0)
            continue;
        for (size_t i = 0; i < _pollfds.size(); ++i) {
            int fd = _pollfds[i].fd;
            short revents = _pollfds[i].revents;
            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(fd);
                _connections.erase(fd);
                _pollfds.erase(_pollfds.begin() + i);
                --i;
                continue;
            }
            // Handle readable events
            if (revents & POLLIN) {
                if (_fdType[fd] == "listen") {
                    if (!acceptConnection(fd))
                        continue;
                }
                else if (_fdType[fd] == "connection") {
                    std::map<int, Connection>::iterator it = _connections.find(fd);
                    if (it == _connections.end())
                        continue;
                    if (!it->second.readRequest()) {
                        close(fd);
                        _connections.erase(it);
                        _pollfds.erase(_pollfds.begin() + i);
                        --i;
                        continue;
                    }
                    // If response is ready, add POLLOUT to events
                    if (it->second.isDone()) {
                        _pollfds[i].events |= POLLOUT;
                    }
                }
            }
            // Handle writable events
            if (revents & POLLOUT) {
                std::map<int, Connection>::iterator it = _connections.find(fd);
                if (it == _connections.end())
                    continue;
                if (it->second.writeResponse()) {
                    // All data sent, remove POLLOUT
                    _pollfds[i].events &= ~POLLOUT;
                    // Optionally close connection if you don't want keep-alive
                    // close(fd);
                    // _connections.erase(it);
                    // _pollfds.erase(_pollfds.begin() + i);
                    // --i;
                }
            }
        }
    }
}


bool WebServ::acceptConnection(int fd) {
    Connection connection(fd);
    int connection_fd = connection.getFd();
    _connections.insert(std::make_pair(connection_fd, connection));
    addPollFd(connection_fd, POLLIN, "connection");
    return true;
}

void WebServ::checkTimeout() {
    time_t now = time(NULL);
    for (size_t i = 0; i < _pollfds.size(); ++i) {
        int fd = _pollfds[i].fd;
        std::map<int, Connection>::iterator it = _connections.find(fd);
        if (it != _connections.end() && now - it->second.getTime() > 60) {
            std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
            close(fd);
            _connections.erase(it);
            _pollfds.erase(_pollfds.begin() + i);
            --i;
        }
    }
}