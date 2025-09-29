#include "WebServer.hpp"

WebServ::WebServ(){/*init default data*/}

WebServ::~WebServ(){
    // loop on pollfds and close all
    // delete connection pointers  
}

WebServ::WebServ(ServerConf& server): _server(server){/*init data*/}

bool WebServ::startServer(ServerConf& server){
    WebServ webserv(server);
    std::vector<std::pair<std::string, std::string> > listens = server.getListen();
    std::vector<std::pair<std::string, std::string> >::iterator it;
    for (it = listens.begin(); it != listens.end(); it++) {
        int server_fd = Socket::StartSocket(it->first, it->second);
        webserv.addPollFd(server_fd, POLLIN, "listen");
        std::cout << "Listening on http" << M" " << it->first << ":" << it->second << B"" << std::endl;
        CHOROUK &&  std::cout << "Server socket  (fd "<< server_fd << ")" << std::endl;
        webserv._listenFds[server_fd] = *it;
    }
    std::cout << B"Document root is " << M" " << server.getRoot() << std::endl;
    std::cout << B"Press Ctrl-C to quit." << B"\n";
    webserv.pollLoop();
    return (true); // check return value value for all functions or remove and use exception!!
}

void WebServ::addPollFd(int fd, short event, const std::string& type){
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = event;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
    _fdType[fd] = type;
}

void WebServ::pollLoop(){
    _cleanRead = false;
    while (true){
        CHOROUK && std::cout << "----------- IN POLLOOP ---------------\n";
        int n = poll(_pollfds.data(), _pollfds.size(), 1000);  // 1-second poll timeout to check timeouts regularly
        if (n == -1){
            perror("Poll failed");
            break; // Only break on fatal poll error
        }
        checkTimeout();
        // if (n == 0) continue;  // no events, continue loop
        
        // Process events - iterate backwards to handle erasing safely
        for (int i = (int)_pollfds.size() - 1; i >= 0; i--){
            CHOROUK && std::cout << "----------- INSIDE POLLOOP ---------------\n";
            // CHOROUK && std::cout << "------- pollfd_size = " << (int)_pollfds.size();
            // CHOROUK && std::cout << "\n";
            int fd = _pollfds[i].fd;
            
                // CHOROUK && std::cout << "----------- INSIDE POLLIN ---------------\n";
            if (_pollfds[i].revents & POLLIN && _fdType[fd] == "listen"){
                    CHOROUK && std::cout << "----------- INSIDE LISTEN ---------------\n";
                    acceptConnection(fd); // Always try to accept new connections
                }
            if ((_pollfds[i].revents & POLLIN) || !_cleanRead){
                if (_fdType[fd] == "connection"){
                    CHOROUK && std::cout << "----------- INSIDE CONNECTION ---------------\n";
                    std::map<int, Connection*>::iterator it = _connections.find(fd);
                    if (it == _connections.end()) continue; // Safety check - connection not found
                    CHOROUK && std::cout << "----------- INSIDE READ ---------------\n";
                    if (!it->second->readRequest()){
                        CHOROUK && std::cout << "----------- READ FAIL ---------------\n";
                        // Connection error - clean up and continue
                        close(fd);
                        delete it->second;
                        _connections.erase(it);
                        _fdType.erase(fd);
                        _pollfds.erase(_pollfds.begin() + i);
                        continue;
                    }
                    
                    if (it->second->isDone()){
                        CHOROUK && std::cout << "----------- READ DONE ---------------\n";
                        _cleanRead = true;
                        // Request complete, switch to write mode
                        // _pollfds[i].events = POLLOUT;
                        // _pollfds[i].revents = 0;
                        // it->second->printRequest(); // to remove!
                    }
                    else {
                        CHOROUK && std::cout << "----------- READ NOT DONE ---------------\n";
                        // Request complete, switch to write mode
                        // _pollfds[i].events = POLLIN;
                        // _pollfds[i].revents = 0;
                        // it->second->printRequest(); // to remove!
                    }
                }
            }
            else if (_pollfds[i].revents & POLLOUT){
                CHOROUK && std::cout << "----------- INSIDE POLLOUT ---------------\n";
                std::map<int, Connection*>::iterator it = _connections.find(fd);
                if (it == _connections.end()) continue; // Safety check
                CHOROUK && std::cout << "----------- INSIDE WRITE ---------------\n";
                it->second->writeResponse();
                CHOROUK && std::cout << "----------- AFTER WRITE ---------------\n";
                if (it->second->isResponseDone()) {
                    CHOROUK && std::cout << "----------- WRITE DONE ---------------\n";
                    // _pollfds[i].events = 0;
                    // _pollfds[i].revents = 0;
                        close(fd);
                        delete it->second;
                        _connections.erase(it);
                        _fdType.erase(fd);
                        _pollfds.erase(_pollfds.begin() + i);
                        continue;
                    // if (it->second->getConnectionHeader() == "close"){ // check if "close" close connection
                    //     // closeConnection();
                    //     CHOROUK && std::cout << "Close connection header fd " << fd << std::endl;
                    //     delete it->second;
                    //     close(fd);
                    //     _connections.erase(it);
                    //     _fdType.erase(fd);
                    //     _pollfds.erase(_pollfds.begin() + i);
                    // }
                    // Response is complete - close the connection and clean up
                    // close(fd);
                    // _connections.erase(it);
                    // _fdType.erase(fd);
                    // _pollfds.erase(_pollfds.begin() + i);
                }
                else {
                    CHOROUK && std::cout << "----------- WRITE NOT DONE ---------------\n";
                    // Response not done (chunked response in progress)
                    // _pollfds[i].events = POLLOUT;
                    // _pollfds[i].revents = 0;
                }

            }
        }
        // After processing all events, continue to next poll cycle
        // The server NEVER stops listening for new connections
    }
}

bool WebServ::acceptConnection(int fd){
    CHOROUK && std::cout << "new connection fd " << fd << " " << _server.getRoot() << std::endl; 
    Connection* connection = new Connection(fd, _server, _listenFds[fd].first, _listenFds[fd].second);
    int connection_fd = connection->getFd();
    _connections.insert(std::make_pair(connection_fd, connection));
    CHOROUK && std::cout << "------- accept fd = " << connection_fd << std::endl;
    // struct pollfd client_pfd;
    // client_pfd.fd = connection_fd;
    // client_pfd.events = POLLIN;
    // client_pfd.revents = 0;
    // _pollfds.push_back(client_pfd);
    _cleanRead = false;
    addPollFd(connection_fd, POLLIN|POLLOUT, "connection"); // to change later by macro

    return (true);
}

void WebServ::checkTimeout(){
    time_t now = time(NULL);
    for (size_t i = 0; i < _pollfds.size(); i++){
        int fd = _pollfds[i].fd;
        if (_fdType[fd] == "listen") continue;
        std::map<int, Connection*>::iterator it = _connections.find(fd);
        if (it != _connections.end() && now - it->second->getTime() > 60){
            std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
            delete it->second;
            close(fd);
            _connections.erase(it);
            _fdType.erase(fd);
            _pollfds.erase(_pollfds.begin() + i);
            i--;
            
        }
    }
}

// void WebServ::closeConnection(int fd){


// }