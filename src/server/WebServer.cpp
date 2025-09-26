#include "WebServer.hpp"

WebServer::WebServer(ServerConf& cfg) : config(cfg) {}

WebServer::~WebServer() {
    for (std::map<int, Connection*>::iterator it = conns.begin(); 
            it != conns.end(); ++it) {
        close(it->first);
        delete it->second;
    }
}

bool WebServer::start() {
    std::vector<std::pair<std::string, std::string> > addresses = config.getListen();
    for (size_t i = 0; i < addresses.size(); ++i) {
        int fd = Socket::StartSocket(addresses[i].first, addresses[i].second);
        if (fd < 0) return false;
        
        addPoll(fd, POLLIN);
        Connection* conn = new Connection(Connection::LISTEN);
        conn->fd = fd;
        conns[fd] = conn;
        
        std::cout << "Listening on " << addresses[i].first 
                    << ":" << addresses[i].second << std::endl;
    }
    return true;
}

void WebServer::run() {
    while (true) {
        int n = poll(&fds[0], fds.size(), 1000);
        if (n < 0) break;
        
        cleanup();
        
        for (int i = static_cast<int>(fds.size()) - 1; i >= 0; i--) {
            if (fds[i].revents == 0) continue;
            
            int fd = fds[i].fd;
            std::map<int, Connection*>::iterator it = conns.find(fd);
            if (it == conns.end()) continue;
            
            Connection* c = it->second;
            c->touch();
            
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                closeConn(fd);
                continue;
            }
            
            switch (c->type) {
            case Connection::LISTEN:
                if (fds[i].revents & POLLIN) {
                    acceptClient(fd);
                }
                break;
                
            case Connection::CLIENT:
                if (fds[i].revents & POLLIN && c->state == Connection::READING) {
                    if (!readRequest(c)) {
                        closeConn(fd);
                    }
                }
                if (fds[i].revents & POLLOUT && c->state == Connection::WRITING) {
                    if (!writeResponse(c)) {
                        closeConn(fd);
                    }
                }
                break;
                
            case Connection::CGI:
                if (fds[i].revents & POLLIN) {
                    if (!readCGI(c)) {
                        finishCGI(c);
                    }
                }
                break;
            }
        }
    }
}


void WebServer::addPoll(int fd, short events) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    fds.push_back(pfd);
}

void WebServer::removePoll(int fd) {
    for (std::vector<struct pollfd>::iterator it = fds.begin(); 
            it != fds.end(); ++it) {
        if (it->fd == fd) {
            fds.erase(it);
            break;
        }
    }
}

void WebServer::modifyPoll(int fd, short events) {
    for (size_t i = 0; i < fds.size(); ++i) {
        if (fds[i].fd == fd) {
            fds[i].events = events;
            break;
        }
    }
}

void WebServer::closeConn(int fd) {
    std::map<int, Connection*>::iterator it = conns.find(fd);
    if (it == conns.end()) return;
    
    Connection* c = it->second;
    
    // Close related CGI if client closes
    if (c->type == Connection::CLIENT) {
        for (std::map<int, Connection*>::iterator cgi_it = conns.begin();
                cgi_it != conns.end(); ++cgi_it) {
            if (cgi_it->second->type == Connection::CGI && 
                cgi_it->second->parent_fd == fd) {
                closeConn(cgi_it->first);
                break;
            }
        }
    }
    
    removePoll(fd);
    close(fd);
    delete c;
    conns.erase(it);
}

void WebServer::cleanup() {
    std::vector<int> expired;
    for (std::map<int, Connection*>::iterator it = conns.begin();
            it != conns.end(); ++it) {
        if (it->second->type != Connection::LISTEN && it->second->expired()) {
            expired.push_back(it->first);
        }
    }
    for (size_t i = 0; i < expired.size(); ++i) {
        closeConn(expired[i]);
    }
}

void WebServer::acceptClient(int listen_fd) {
    int fd = accept(listen_fd, NULL, NULL);
    if (fd < 0) return;
    
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    
    Connection* c = new Connection(Connection::CLIENT);
    c->fd = fd;
    c->req = new Request();
    
    conns[fd] = c;
    addPoll(fd, POLLIN);
}

bool WebServer::readRequest(Connection* c) {
    char buf[4096];
    ssize_t n = read(c->fd, buf, sizeof(buf));
    
    if (n <= 0) {
        return (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    }
    
    c->read_buf.append(buf, n);
    
    if (c->req->parse(c->read_buf)) {
        c->state = Connection::PROCESSING;
        modifyPoll(c->fd, 0); // Remove POLLIN
        return processRequest(c);
    }
    
    return true;
}

bool WebServer::processRequest(Connection* c) {
    if (c->req->isCGI()) {
        return startCGI(c);
    }
    
    c->resp = new Response();
    c->resp->handle(*c->req, config);
    c->write_buf = c->resp->build();
    c->write_pos = 0;
    c->state = Connection::WRITING;
    modifyPoll(c->fd, POLLOUT);
    return true;
}

bool WebServer::startCGI(Connection* c) {
    int pipes[2];
    if (pipe(pipes) < 0) return false;
    
    pid_t pid = fork();
    if (pid < 0) {
        close(pipes[0]); 
        close(pipes[1]);
        return false;
    }
    
    if (pid == 0) {
        // Child: setup and exec CGI
        close(pipes[0]);
        dup2(pipes[1], STDOUT_FILENO);
        dup2(pipes[1], STDERR_FILENO);
        close(pipes[1]);
        
        CGI::executeCGI(*c->req, config);
        exit(1);
    }
    
    // Parent: setup CGI connection
    close(pipes[1]);
    fcntl(pipes[0], F_SETFL, fcntl(pipes[0], F_GETFL) | O_NONBLOCK);
    
    Connection* cgi = new Connection(Connection::CGI);
    cgi->fd = pipes[0];
    cgi->parent_fd = c->fd;
    cgi->pid = pid;
    
    conns[pipes[0]] = cgi;
    addPoll(pipes[0], POLLIN);
    
    return true;
}

bool WebServer::readCGI(Connection* cgi) {
    char buf[4096];
    ssize_t n = read(cgi->fd, buf, sizeof(buf));
    
    if (n <= 0) {
        return (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    }
    
    cgi->read_buf.append(buf, n);
    return true;
}

void WebServer::finishCGI(Connection* cgi) {
    std::map<int, Connection*>::iterator client_it = conns.find(cgi->parent_fd);
    if (client_it == conns.end()) {
        closeConn(cgi->fd);
        return;
    }
    
    Connection* client = client_it->second;
    waitpid(cgi->pid, NULL, WNOHANG);
    
    client->resp = new Response();
    client->resp->fromCGI(cgi->read_buf);
    client->write_buf = client->resp->build();
    client->write_pos = 0;
    client->state = Connection::WRITING;
    modifyPoll(client->fd, POLLOUT);
    
    closeConn(cgi->fd);
}

bool WebServer::writeResponse(Connection* c) {
    if (c->write_pos >= c->write_buf.size()) {
        closeConn(c->fd);
        return false;
    }
    
    const char* data = c->write_buf.data() + c->write_pos;
    size_t len = c->write_buf.size() - c->write_pos;
    
    ssize_t n = write(c->fd, data, len);
    if (n <= 0) {
        return (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    }
    
    c->write_pos += n;
    
    if (c->write_pos >= c->write_buf.size()) {
        closeConn(c->fd);
        return false;
    }
    
    return true;
}

int WebServer::createListenSocket(const std::string& host, const std::string& port){

}

// void WebServer::execCGI(const Request& req, ServerConf& config){
    
// }


// WebServ::WebServ(){/*init default data*/}

// WebServ::~WebServ(){
//     // loop on pollfds and close all
//     // delete connection pointers  
// }

// WebServ::WebServ(ServerConf& server): _server(server), _listens(server.getListen()){/*init data*/}

// bool WebServ::startServer(ServerConf& server){
//     WebServ webserv(server);

//     std::vector<std::pair<std::string, std::string> >::iterator it;
//     for (it = webserv._listens.begin(); it != webserv._listens.end(); it++) {
//         int server_fd = Socket::StartSocket(it->first, it->second);
//         webserv.addPollFd(server_fd, POLLIN, "listen");
//         std::cout << "Listening on http" << M" " << it->first << ":" << it->second << std::endl;
//         std::cout << B"Document root is " << M" " << server.getRoot() << std::endl;
//         std::cout << B"Press Ctrl-C to quit." << B"\n";
//         CHOROUK &&  std::cout << "Server socket  (fd "<< server_fd << ")" << std::endl;
//     }
//     webserv.pollLoop();
//     return (true); // check return value value for all functions or remove and use exception!!
// }

// void WebServ::addPollFd(int fd, short event, const std::string& type){
//     struct pollfd pfd;
//     pfd.fd = fd;
//     pfd.events = event;
//     pfd.revents = 0;
//     _pollfds.push_back(pfd);
//     _fdType[fd] = type;
// }

// void WebServ::pollLoop(){
//     _cleanRead = false;
//     _cleanWrite = false;
//     while (true){
//         CHOROUK && std::cout << "----------- IN POLLOOP ---------------\n";
//         int n = poll(_pollfds.data(), _pollfds.size(), 1000);  // 1-second poll timeout to check timeouts regularly
//         if (n == -1){
//             perror("Poll failed");
//             break; // Only break on fatal poll error
//         }
//         checkTimeout();
//         // if (n == 0) continue;  // no events, continue loop
        
//         // Process events - iterate backwards to handle erasing safely
//         for (int i = (int)_pollfds.size() - 1; i >= 0; i--){
//             CHOROUK && std::cout << "----------- INSIDE POLLOOP ---------------\n";
//             // CHOROUK && std::cout << "------- pollfd_size = " << (int)_pollfds.size();
//             // CHOROUK && std::cout << "\n";
//             int fd = _pollfds[i].fd;
            
//                 // CHOROUK && std::cout << "----------- INSIDE POLLIN ---------------\n";
//             if (_pollfds[i].revents & POLLIN && _fdType[fd] == "listen"){
//                     CHOROUK && std::cout << "----------- INSIDE LISTEN ---------------\n";
//                     acceptConnection(fd); // Always try to accept new connections
//                 }
//             if ((_pollfds[i].revents & POLLIN) || !_cleanRead){


//                 // if (_fdType[fd] == "cgi"){
//                 //     CHOROUK && std::cout << "----------- INSIDE CGI ---------------\n";
//                 //     std::map<int, Connection*>::iterator it = _cgis.find(fd);
//                 //     if (it == _cgis.end()) continue; // Safety check - connection not found
//                 //     CHOROUK && std::cout << "----------- INSIDE READ CGI ---------------\n";


//                 //     if (!it->second->writeResponse()){
//                 //         CHOROUK && std::cout << "----------- READ FAIL ---------------\n";
//                 //         // Connection error - clean up and continue
//                 //         close(fd);
//                 //         delete it->second;
//                 //         _cgis.erase(fd);
//                 //         _connections.erase(it);
//                 //         _fdType.erase(fd);
//                 //         _pollfds.erase(_pollfds.begin() + i);
//                 //         continue;
//                 //     }

//                 //     // std::string cgi_response = it->second->sendResponse();
//                 //     // if (cgi_response.empty()){
//                 //     //     CHOROUK && std::cout << "----------- READ CGI FAIL ---------------\n";
//                 //     //     // Connection error - clean up and continue
//                 //     //     close(fd);
//                 //     //     delete it->second;
//                 //     //     _connections.erase(it);
//                 //     //     _fdType.erase(fd);
//                 //     //     _pollfds.erase(_pollfds.begin() + i);
//                 //     //     _cgis.erase(fd);
//                 //     //     continue;
//                 //     // }
//                 //     // CHOROUK && std::cout << "----------- READ CGI DONE ---------------\n";
//                 //     // // Request complete, switch to write mode
//                 //     // // _pollfds[i].events = POLLOUT;
//                 //     // // _pollfds[i].revents = 0;
//                 //     // // it->second->printRequest(); // to remove!
//                 //     // // Now set the response in the connection and prepare for writing
//                 //     // it->second->setCgiFd(-1); // reset cgi fd in connection
//                 //     // _cgis.erase(fd); // remove from cgi map
//                 //     // close(fd); // close the cgi pipe fd
//                 //     // it->second->sendGetResponse(*(it->second->_request), _server); // to change later by macro
//                 //     // _pollfds[i].events = POLLOUT;
//                 //     // _pollfds[i].revents = 0;
//                 // }


//                 if (_fdType[fd] == "connection"){
//                     CHOROUK && std::cout << "----------- INSIDE CONNECTION ---------------\n";
//                     std::map<int, Connection*>::iterator it = _connections.find(fd);
//                     if (it == _connections.end()) continue; // Safety check - connection not found
//                     CHOROUK && std::cout << "----------- INSIDE READ ---------------\n";
//                     // if (it->second->getCgiFd()!= -1){
//                     //     addPollFd(it->second->getCgiFd(), POLLIN, "cgi");
//                     //     _cgis[it->second->getCgiFd()] = it->second;
//                     //     it->second->setCgiFd(-1); // reset to avoid adding multiple times
//                     // }
//                     if (!it->second->readRequest()){
//                         CHOROUK && std::cout << "----------- READ FAIL ---------------\n";
//                         // Connection error - clean up and continue
//                         close(fd);
//                         delete it->second;
//                         _connections.erase(it);
//                         _fdType.erase(fd);
//                         _pollfds.erase(_pollfds.begin() + i);
//                         continue;
//                     }
                    
//                     if (it->second->isDone()){
//                         CHOROUK && std::cout << "----------- READ DONE ---------------\n";
//                         _cleanRead = true;
//                         // Request complete, switch to write mode
//                         // _pollfds[i].events = POLLOUT;
//                         // _pollfds[i].revents = 0;
//                         // it->second->printRequest(); // to remove!
//                     }
//                     else {
//                         CHOROUK && std::cout << "----------- READ NOT DONE ---------------\n";
//                         // Request complete, switch to write mode
//                         // _pollfds[i].events = POLLIN;
//                         // _pollfds[i].revents = 0;
//                         // it->second->printRequest(); // to remove!
//                     }
//                 }
//             }
//             else if ((_pollfds[i].revents & POLLOUT) || (_cleanRead && !_cleanWrite)){

//                 CHOROUK && std::cout << "----------- INSIDE POLLOUT ---------------\n";
//                 std::map<int, Connection*>::iterator it = _connections.find(fd);
//                 if (it == _connections.end()) continue; // Safety check

//                 if (_fdType[fd] == "cgi"){
//                     CHOROUK && std::cout << "----------- INSIDE CGI ---------------\n";
//                     std::map<int, Connection*>::iterator it = _cgis.find(fd);
//                     if (it == _cgis.end()) continue; // Safety check - connection not found

//                     CHOROUK && std::cout << "--cgi fd = " << fd;
//                     CHOROUK && std::cout << "\n";
//                     CHOROUK && std::cout << "----------- INSIDE write CGI ---------------\n";

//                     if (!it->second->writeResponse()){
//                         // CHOROUK && std::cout << "----------- READ FAIL ---------------\n";
//                         // Connection error - clean up and continue
//                         close(fd);
//                         delete it->second;
//                         _cgis.erase(fd);
//                         _connections.erase(it);
//                         _fdType.erase(fd);
//                         _pollfds.erase(_pollfds.begin() + i);
//                         _cleanWrite = true;
//                         continue;
//                     }

//                     // std::string cgi_response = it->second->sendResponse();
//                     // if (cgi_response.empty()){
//                     //     CHOROUK && std::cout << "----------- READ CGI FAIL ---------------\n";
//                     //     // Connection error - clean up and continue
//                     //     close(fd);
//                     //     delete it->second;
//                     //     _connections.erase(it);
//                     //     _fdType.erase(fd);
//                     //     _pollfds.erase(_pollfds.begin() + i);
//                     //     _cgis.erase(fd);
//                     //     continue;
//                     // }
//                     // CHOROUK && std::cout << "----------- READ CGI DONE ---------------\n";
//                     // // Request complete, switch to write mode
//                     // // _pollfds[i].events = POLLOUT;
//                     // // _pollfds[i].revents = 0;
//                     // // it->second->printRequest(); // to remove!
//                     // // Now set the response in the connection and prepare for writing
//                     // it->second->setCgiFd(-1); // reset cgi fd in connection
//                     // _cgis.erase(fd); // remove from cgi map
//                     // close(fd); // close the cgi pipe fd
//                     // it->second->sendGetResponse(*(it->second->_request), _server); // to change later by macro
//                     // _pollfds[i].events = POLLOUT;
//                     // _pollfds[i].revents = 0;
//                 }
//                 CHOROUK && std::cout << "----------- INSIDE WRITE ---------------\n";
//                 it->second->writeResponse();
//                 CHOROUK && std::cout << "----------- AFTER WRITE ---------------\n";
//                 std::cout << "------------------------------------ cgi fd = " << it->second->getCgiFd() << "\n";
//                 if (it->second->cgiDone()){
//                     CHOROUK && std::cout << "----------- CGI POLLOUT ---------------\n";
//                     addPollFd(it->second->getCgiFd(), POLLOUT, "cgi");
//                     _cgis[it->second->getCgiFd()] = it->second;
//                     // it->second->setCgiFd(-1); // reset to avoid adding multiple times
//                     continue ;
//                 }
//                 else if (it->second->isResponseDone()) {
//                     CHOROUK && std::cout << "----------- WRITE DONE ---------------\n";
//                     _cleanWrite = true;
//                     // _pollfds[i].events = 0;
//                     // _pollfds[i].revents = 0;
//                         close(fd);
//                         delete it->second;
//                         _connections.erase(it);
//                         _fdType.erase(fd);
//                         _pollfds.erase(_pollfds.begin() + i);
//                         continue;
//                     // if (it->second->getConnectionHeader() == "close"){ // check if "close" close connection
//                     //     // closeConnection();
//                     //     CHOROUK && std::cout << "Close connection header fd " << fd << std::endl;
//                     //     delete it->second;
//                     //     close(fd);
//                     //     _connections.erase(it);
//                     //     _fdType.erase(fd);
//                     //     _pollfds.erase(_pollfds.begin() + i);
//                     // }
//                     // Response is complete - close the connection and clean up
//                     // close(fd);
//                     // _connections.erase(it);
//                     // _fdType.erase(fd);
//                     // _pollfds.erase(_pollfds.begin() + i);
//                 }
//                 else {
//                     CHOROUK && std::cout << "----------- WRITE NOT DONE ---------------\n";
//                     // Response not done (chunked response in progress)
//                     // _pollfds[i].events = POLLOUT;
//                     // _pollfds[i].revents = 0;
//                 }

//             }
//         }
//         // After processing all events, continue to next poll cycle
//         // The server NEVER stops listening for new connections
//     }
// }

// bool WebServ::acceptConnection(int fd){
//     CHOROUK && std::cout << "new connection fd " << fd << " " << _server.getRoot() << std::endl; 
//     Connection* connection = new Connection(fd, _server);
//     int connection_fd = connection->getFd();
//     _connections.insert(std::make_pair(connection_fd, connection));
//     CHOROUK && std::cout << "------- accept fd = " << connection_fd << std::endl;
//     // struct pollfd client_pfd;
//     // client_pfd.fd = connection_fd;
//     // client_pfd.events = POLLIN;
//     // client_pfd.revents = 0;
//     // _pollfds.push_back(client_pfd);
//     _cleanRead = false;
//     addPollFd(connection_fd, POLLIN|POLLOUT, "connection"); // to change later by macro

//     return (true);
// }

// void WebServ::checkTimeout(){
//     time_t now = time(NULL);
//     for (size_t i = 0; i < _pollfds.size(); i++){
//         int fd = _pollfds[i].fd;
//         if (_fdType[fd] == "listen") continue;
//         std::map<int, Connection*>::iterator it = _connections.find(fd);
//         if (it != _connections.end() && now - it->second->getTime() > 60){
//             std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
//             delete it->second;
//             close(fd);
//             _connections.erase(it);
//             _fdType.erase(fd);
//             _pollfds.erase(_pollfds.begin() + i);
//             i--;
            
//         }
//     }
// }

// // void WebServ::closeConnection(int fd){


// // }