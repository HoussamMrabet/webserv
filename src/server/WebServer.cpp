#include "WebServer.hpp"

    
fdInfo::fdInfo(Type t):    fd(-1), type(t), state(READING), 
                            req(NULL), resp(NULL), write_pos(0), 
                            activity(time(NULL)), parent_fd(-1), pid(-1) {}

fdInfo::~fdInfo(){ 
    delete req; 
    delete resp;
    req = NULL;
    resp = NULL;
    if (pid > 0) { 
        kill(pid, SIGTERM); 
        waitpid(pid, NULL, WNOHANG); 
    } 
}
void fdInfo::timer(){ 
    activity = time(NULL); 
}

bool fdInfo::timedout(){ 
    return (time(NULL) - activity > TIMEOUT); 
}

void WebServ::addPoll(int fd, short events) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    fds.push_back(pfd);
}

void WebServ::cleanup() {
    std::vector<int> expired;
    for (std::map<int, fdInfo*>::iterator it = conns.begin();
            it != conns.end(); ++it) {
        if (it->second->type != fdInfo::LISTEN && it->second->timedout()) {
            expired.push_back(it->first);
        }
    }
    for (size_t i = 0; i < expired.size(); ++i) {
        closeConn(expired[i]);
    }
}

void WebServ::closeConn(int fd) {
    std::map<int, fdInfo*>::iterator it = conns.find(fd);
    if (it == conns.end()) return;
    
    fdInfo* c = it->second;
    
    // Close related CGI if client closes
    if (c->type == fdInfo::CLIENT) {
        for (std::map<int, fdInfo*>::iterator cgi_it = conns.begin();
                cgi_it != conns.end(); ++cgi_it) {
            if (cgi_it->second->type == fdInfo::CGI && 
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

void WebServ::removePoll(int fd) {
    for (std::vector<struct pollfd>::iterator it = fds.begin(); 
            it != fds.end(); ++it) {
        if (it->fd == fd) {
            fds.erase(it);
            break;
        }
    }
}

void WebServ::modifyPoll(int fd, short events) {
    for (size_t i = 0; i < fds.size(); ++i) {
        if (fds[i].fd == fd) {
            fds[i].events = events;
            break;
        }
    }
}

void WebServ::acceptClient(int listen_fd) {
    int fd = accept(listen_fd, NULL, NULL);
    if (fd < 0) return;
    
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    
    fdInfo* c = new fdInfo(fdInfo::CLIENT);
    c->fd = fd;
    c->req = new Request();
    
    conns[fd] = c;
    addPoll(fd, POLLIN);
}

bool WebServ::readRequest(fdInfo* c) {
    char buf[4096];
    ssize_t n = read(c->fd, buf, sizeof(buf));
    
    // if (n <= 0) {
    //     return (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    // }
    if (n > 0)
    {
        c->read_buf.append(buf, n);
        c->req->parseRequest(c->read_buf);
        c->state = fdInfo::PROCESSING;
        c->read_buf.clear();
        c->timer();
        // updateTimout();  // update activity timestamp
        // else continue;
    }
    else
    {
        c->req->parseRequest();
    }
    if (c->req->isDone()){
        // c->state = fdInfo::WRITING;
        // modifyPoll(c->fd, POLLOUT); // Remove POLLIN
        c->state = fdInfo::PROCESSING;
        modifyPoll(c->fd, 0); // Remove POLLIN
        return processRequest(c);
    }
    else
        c->state = fdInfo::READING;
    // c->read_buf.append(buf, n);

    // if (c->req->parse(c->read_buf)) {
    //     return processRequest(c);
    // }
    
    return true;
}

bool WebServ::processRequest(fdInfo* c) {
    if (c->req->isCGI()) {
        return startCGI(c);
    }
    
    c->resp = new Response2();
    c->resp->handle(*c->req, _server);
    c->write_buf = c->resp->build();
    // c->write_buf = "default response"; // to remove later
    c->write_pos = 0;
    c->state = fdInfo::WRITING;
    modifyPoll(c->fd, POLLOUT);
    return true;
}

bool WebServ::startCGI(fdInfo* c) {
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
        
        execCGI(*c->req, _server);
        // CGI::executeCGI(*c->req, _server);
        exit(1);
    }
    
    // Parent: setup CGI fdInfo
    close(pipes[1]);
    fcntl(pipes[0], F_SETFL, fcntl(pipes[0], F_GETFL) | O_NONBLOCK);
    
    fdInfo* cgi = new fdInfo(fdInfo::CGI);
    cgi->fd = pipes[0];
    cgi->parent_fd = c->fd;
    cgi->pid = pid;
    
    conns[pipes[0]] = cgi;
    addPoll(pipes[0], POLLIN);
    
    return true;
}

void WebServ::execCGI(const Request& req, ServerConf& config) {
    // This runs in child process after fork()

    std::string uri = req.getUri();
    std::string location = req.getLocation();
    
    // Get CGI configuration
    std::map<std::string, LocationConf> locations = config.getLocations();
    if (locations.find(location) == locations.end()) {
        exit(1);
    }
    
    LocationConf loc_conf = locations[location];
    std::map<std::string, std::string> cgi_map = loc_conf.getCgi();
    std::string cgi_type = req.getCgiType();
    
    if (cgi_map.find(cgi_type) == cgi_map.end()) {
        exit(1);
    }
    
    std::string interpreter = cgi_map[cgi_type];
    
    // Build script path
    std::string root = loc_conf.getRoot().empty() ? "./www" : loc_conf.getRoot();
    std::string script_path = root + uri;
    
    // Validate paths
    if (access(script_path.c_str(), R_OK) != 0 || 
        access(interpreter.c_str(), X_OK) != 0 ||
        script_path.find("../") != std::string::npos) {
        exit(1);
    }
    
    // Setup environment
    std::vector<std::string> env_vars;
    std::vector<char*> env_ptrs;
    
    // Standard CGI environment variables
    env_vars.push_back("REQUEST_METHOD=" + req.getMethod());
    env_vars.push_back("SCRIPT_NAME=" + uri);
    env_vars.push_back("SCRIPT_FILENAME=" + script_path);
    env_vars.push_back("QUERY_STRING=" + req.getUriQueries());
    env_vars.push_back("CONTENT_TYPE=" + req.getHeader("Content-Type"));
    env_vars.push_back("CONTENT_LENGTH=" + req.getHeader("Content-Length"));
    env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env_vars.push_back("SERVER_SOFTWARE=webserv/1.0");
    env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env_vars.push_back("REMOTE_ADDR=" + req.getHost());
    
    // Add HTTP headers as environment variables
    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        
        if (it->first == "content-type" || it->first == "content-length") {
            continue; // Already added above
        }
        
        std::string env_name = "HTTP_" + it->first;
        // Convert to uppercase and replace - with _
        for (size_t i = 5; i < env_name.size(); ++i) {
            if (env_name[i] == '-') {
                env_name[i] = '_';
            } else {
                env_name[i] = toupper(env_name[i]);
            }
        }
        env_vars.push_back(env_name + "=" + it->second);
    }
    
    // Convert to char* array
    for (size_t i = 0; i < env_vars.size(); ++i) {
        env_ptrs.push_back(const_cast<char*>(env_vars[i].c_str()));
    }
    env_ptrs.push_back(NULL);
    
    // Setup argv
    char* argv[3];
    argv[0] = const_cast<char*>(interpreter.c_str());
    argv[1] = const_cast<char*>(script_path.c_str());
    argv[2] = NULL;
    
    // Execute CGI script
    execve(interpreter.c_str(), argv, &env_ptrs[0]);
    
    // If we get here, execve failed
    exit(1);
}

bool WebServ::readCGI(fdInfo* cgi) {
    char buf[4096];
    ssize_t n = read(cgi->fd, buf, sizeof(buf));
    
    if (n <= 0) {
        return (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    }
    
    cgi->read_buf.append(buf, n);
    return true;
}

void WebServ::finishCGI(fdInfo* cgi) {
    std::map<int, fdInfo*>::iterator client_it = conns.find(cgi->parent_fd);
    if (client_it == conns.end()) {
        closeConn(cgi->fd);
        return;
    }
    
    fdInfo* client = client_it->second;
    waitpid(cgi->pid, NULL, WNOHANG);
    
    client->resp = new Response2();
    client->resp->fromCGI(cgi->read_buf);
    client->write_buf = client->resp->build();
    // client->write_buf = "CGI response"; // to remove later
    client->write_pos = 0;
    client->state = fdInfo::WRITING;
    modifyPoll(client->fd, POLLOUT);
    
    closeConn(cgi->fd);
}

bool WebServ::writeResponse(fdInfo* c) {
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














    
// WebServ::WebServ(){/*init default data*/}

WebServ::~WebServ(){
    // loop on pollfds and close all
    // delete connection pointers  
    for (std::map<int, fdInfo*>::iterator it = conns.begin(); it != conns.end(); ++it) {
            close(it->first);
            delete it->second;
    }
}

WebServ::WebServ(ServerConf& server): _server(server), _listens(server.getListen()){/*init data*/}

bool WebServ::startServer(ServerConf& server){
    WebServ webserv(server);

    std::vector<std::pair<std::string, std::string> >::iterator it;
    for (it = webserv._listens.begin(); it != webserv._listens.end(); it++) {
        int server_fd = Socket::StartSocket(it->first, it->second);
        if (server_fd < 0) return (false); // throw 
        // webserv.addPollFd(server_fd, POLLIN, "listen");
        /***********************/
        webserv.addPoll(server_fd, POLLIN);
        fdInfo* conn = new fdInfo(fdInfo::LISTEN);
        conn->fd = server_fd;
        webserv.conns[server_fd] = conn;
        // std::cout << "Listening on " << it->first << ":" << it->second << std::endl;
        /***********************/


        std::cout << "Listening on http" << M" " << it->first << ":" << it->second << std::endl;
        std::cout << B"Document root is " << M" " << server.getRoot() << std::endl;
        std::cout << B"Press Ctrl-C to quit." << B"\n";
        CHOROUK &&  std::cout << "Server socket  (fd "<< server_fd << ")" << std::endl;
    }
    webserv.pollLoop();
    return (true); // check return value value for all functions or remove and use exception!!
}

// void WebServ::addPollFd(int fd, short event, const std::string& type){
//     struct pollfd pfd;
//     pfd.fd = fd;
//     pfd.events = event;
//     pfd.revents = 0;
//     _pollfds.push_back(pfd);
//     _fdType[fd] = type;
// }

void WebServ::pollLoop(){
    // _cleanRead = false;
    while (true){
        CHOROUK && std::cout << "----------- IN POLLOOP ---------------\n";
        // int n = poll(_pollfds.data(), _pollfds.size(), 1000);  // 1-second poll timeout to check timeouts regularly
        // if (n == -1){
        //     perror("Poll failed");
        //     break; // Only break on fatal poll error
        // }
        // checkTimeout();
        
        
        /*************************/
        int m = poll(&fds[0], fds.size(), 1000);
        if (m < 0) break;
        cleanup();

        for (int i = static_cast<int>(fds.size()) - 1; i >= 0; i--) {
            // if (fds[i].revents == 0) continue;
            
            int fd = fds[i].fd;
            std::map<int, fdInfo*>::iterator it = conns.find(fd);
            if (it == conns.end()) continue;
            
            fdInfo* c = it->second;
            c->timer();
            
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                closeConn(fd);
                continue;
            }
            
            switch (c->type) {
            case fdInfo::LISTEN:
                if (fds[i].revents & POLLIN) {
                    acceptClient(fd);
                }
                break;
                
            case fdInfo::CLIENT:
                if (fds[i].revents & POLLIN || c->state == fdInfo::READING) {
                    if (!readRequest(c)) {
                        closeConn(fd);
                    }
                }
                if (fds[i].revents & POLLOUT && c->state == fdInfo::WRITING) {
                    if (!writeResponse(c)) {
                        closeConn(fd);
                    }
                }
                break;
                
            case fdInfo::CGI:
                if (fds[i].revents & POLLIN) {
                    if (!readCGI(c)) {
                        finishCGI(c);
                    }
                }
                break;
            }
        }


        /*************************/
        // if (n == 0) continue;  // no events, continue loop
        
        // // Process events - iterate backwards to handle erasing safely
        // for (int i = (int)_pollfds.size() - 1; i >= 0; i--){
        //     CHOROUK && std::cout << "----------- INSIDE POLLOOP ---------------\n";
        //     // CHOROUK && std::cout << "------- pollfd_size = " << (int)_pollfds.size();
        //     // CHOROUK && std::cout << "\n";
        //     int fd = _pollfds[i].fd;
            
        //         // CHOROUK && std::cout << "----------- INSIDE POLLIN ---------------\n";
        //     if (_pollfds[i].revents & POLLIN && _fdType[fd] == "listen"){
        //             CHOROUK && std::cout << "----------- INSIDE LISTEN ---------------\n";
        //             acceptConnection(fd); // Always try to accept new connections
        //         }
        //     if ((_pollfds[i].revents & POLLIN) || !_cleanRead){
        //         if (_fdType[fd] == "connection"){
        //             CHOROUK && std::cout << "----------- INSIDE CONNECTION ---------------\n";
        //             std::map<int, Connection*>::iterator it = _connections.find(fd);
        //             if (it == _connections.end()) continue; // Safety check - connection not found
        //             CHOROUK && std::cout << "----------- INSIDE READ ---------------\n";
        //             if (!it->second->readRequest()){
        //                 CHOROUK && std::cout << "----------- READ FAIL ---------------\n";
        //                 // Connection error - clean up and continue
        //                 close(fd);
        //                 delete it->second;
        //                 _connections.erase(it);
        //                 _fdType.erase(fd);
        //                 _pollfds.erase(_pollfds.begin() + i);
        //                 continue;
        //             }
                    
        //             if (it->second->isDone()){
        //                 CHOROUK && std::cout << "----------- READ DONE ---------------\n";
        //                 _cleanRead = true;
        //                 // Request complete, switch to write mode
        //                 // _pollfds[i].events = POLLOUT;
        //                 // _pollfds[i].revents = 0;
        //                 // it->second->printRequest(); // to remove!
        //             }
        //             else {
        //                 CHOROUK && std::cout << "----------- READ NOT DONE ---------------\n";
        //                 // Request complete, switch to write mode
        //                 // _pollfds[i].events = POLLIN;
        //                 // _pollfds[i].revents = 0;
        //                 // it->second->printRequest(); // to remove!
        //             }
        //         }
        //     }
        //     else if (_pollfds[i].revents & POLLOUT){
        //         CHOROUK && std::cout << "----------- INSIDE POLLOUT ---------------\n";
        //         std::map<int, Connection*>::iterator it = _connections.find(fd);
        //         if (it == _connections.end()) continue; // Safety check
        //         CHOROUK && std::cout << "----------- INSIDE WRITE ---------------\n";
        //         it->second->writeResponse();
        //         CHOROUK && std::cout << "----------- AFTER WRITE ---------------\n";
        //         if (it->second->isResponseDone()) {
        //             CHOROUK && std::cout << "----------- WRITE DONE ---------------\n";
        //             // _pollfds[i].events = 0;
        //             // _pollfds[i].revents = 0;
        //                 close(fd);
        //                 delete it->second;
        //                 _connections.erase(it);
        //                 _fdType.erase(fd);
        //                 _pollfds.erase(_pollfds.begin() + i);
        //                 continue;
        //             // if (it->second->getConnectionHeader() == "close"){ // check if "close" close connection
        //             //     // closeConnection();
        //             //     CHOROUK && std::cout << "Close connection header fd " << fd << std::endl;
        //             //     delete it->second;
        //             //     close(fd);
        //             //     _connections.erase(it);
        //             //     _fdType.erase(fd);
        //             //     _pollfds.erase(_pollfds.begin() + i);
        //             // }
        //             // Response is complete - close the connection and clean up
        //             // close(fd);
        //             // _connections.erase(it);
        //             // _fdType.erase(fd);
        //             // _pollfds.erase(_pollfds.begin() + i);
        //         }
        //         else {
        //             CHOROUK && std::cout << "----------- WRITE NOT DONE ---------------\n";
        //             // Response not done (chunked response in progress)
        //             // _pollfds[i].events = POLLOUT;
        //             // _pollfds[i].revents = 0;
        //         }

            // }
        // }
        // // After processing all events, continue to next poll cycle
        // // The server NEVER stops listening for new connections
    }
}

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

// void WebServ::closeConnection(int fd){


// }