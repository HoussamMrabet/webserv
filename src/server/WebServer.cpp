#include "WebServer.hpp"

WebServ::WebServ(){/*init default data*/}

WebServ::WebServ(std::map<Listen, ServerBlock >& serverBlocks): _serverBlocks(serverBlocks){/*init data*/}

bool WebServ::startServer(std::map<Listen, ServerBlock >& serverBlocks){
    WebServ webserv(serverBlocks);
    
    std::map<Listen, ServerBlock >::iterator it;
    for (it = webserv._serverBlocks.begin(); it != webserv._serverBlocks.end(); it++) {
        int server_fd = Socket::StartSocket(it->first.getHost(), it->first.getPort());
        webserv.addPollFd(server_fd, POLLIN, "listen");
        std::cout << "Listening on " << it->first.getHost() << ":" << it->first.getPort() << "..." << std::endl;
    }
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
    while (true){
        int n = poll(_pollfds.data(), _pollfds.size(), 1000);  // 1-second poll timeout to check timeouts regularly
        if (n == -1){
            perror("Poll failed");
            break;
        }
        checkTimeout();
        if (n == 0) continue;  // no events, continue loop
        for (size_t i = 0; i < _pollfds.size(); i++){
            int fd = _pollfds[i].fd;
            if (_pollfds[i].revents & POLLIN){
                // if (newConnection(fd)){
                if (_fdType[fd] == "listen"){
                // if (isListening(fd)){
                    if (!acceptConnection(fd))
                        continue;
                }
                else if (_fdType[fd] == "connection"){
                // else if (isConnection(fd)){
                    std::map<int, Connection>::iterator it = _connections.find(fd);
                    if (!it->second.readRequest()){
                        close(fd);
                        _connections.erase(it);
                        _pollfds.erase(_pollfds.begin() + i);
                        i--;
                        break;
                    }
                    // std::cout << "HERE!!!" << it->second.isDone() << std::endl;
                    // if (it->second.isDone()){
                    //     it->second.printRequest(); // to remove!
                        it->second.writeResponse();
                        // it->second.cgiResponse(pipe_fdout, pipe_fdin); // start pipefds here and add to pollfds 
                    // }
                }
            }
        }
    }
}

bool WebServ::acceptConnection(int fd){
    Connection connection(fd);
    int connection_fd = connection.getFd();
    _connections.insert(std::make_pair(connection_fd, connection));
    // struct pollfd client_pfd;
    // client_pfd.fd = connection_fd;
    // client_pfd.events = POLLIN;
    // client_pfd.revents = 0;
    // _pollfds.push_back(client_pfd);
    addPollFd(connection_fd, POLLIN, "connection"); // to change later by macro

    return (true);
}

void WebServ::checkTimeout(){
    time_t now = time(NULL);
    for (size_t i = 0; i < _pollfds.size(); i++) {
        int fd = _pollfds[i].fd;
        std::map<int, Connection>::iterator it = _connections.find(fd);
        if (it != _connections.end() && now - it->second.getTime() > 60){
            std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
            close(fd);
            _connections.erase(it);
            _pollfds.erase(_pollfds.begin() + i);
            i--;
            
        }
    }
}