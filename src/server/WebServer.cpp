#include "WebServ.hpp"

bool WebServ::_runServer = true;

WebServ::WebServ(){}

WebServ::~WebServ(){
    for (int i = (int)_pollfds.size() - 1; i >= 0; i--)
        close(_pollfds[i].fd);
    _pollfds.clear();
}

WebServ::WebServ(ServerConf& server): _server(server) {}

bool WebServ::startServer(ServerConf& server){
    WebServ webserv(server);
    std::vector<std::pair<std::string, std::string> > listens = server.getListen();
    std::vector<std::pair<std::string, std::string> >::iterator it;
    for (it = listens.begin(); it != listens.end(); it++) {
        try{
            int server_fd = Socket::StartSocket(it->first, it->second);
            webserv.addPollFd(server_fd, POLLIN, "listen");
            std::cout << "Listening on http" << M" " << it->first << ":" << it->second << B"" << std::endl;
            webserv._listenFds[server_fd] = *it;
        }
        catch (const std::exception& e){
            std::cerr << R"Error: " << e.what() << B"\n";
        }
    }
    if (webserv._pollfds.size() == 0){
        std::cerr << R"All listens failed. Server closed.\n";
        exit(1);
    }
    std::cout << B"Document root is " << M" " << server.getRoot() << std::endl;
    std::cout << B"Press Ctrl-C to quit." << B"\n";
    webserv.pollLoop();
    return (true);
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
    while (_runServer){
        int n = poll(_pollfds.data(), _pollfds.size(), 1000);
        
        if (n == -1) {
            if (errno == EINTR) {
                if (!_runServer) {
                    std::cout << "Server shutting down...\n";
                    break;
                }
                continue;
            }
            std::cerr << R"Error: Poll failed" << B"\n";
            break;
        }

        checkTimeout();
        
        // Process events in reverse order to safely erase while iterating
        for (int i = (int)_pollfds.size() - 1; i >= 0; i--){
            int fd = _pollfds[i].fd;
            short revents = _pollfds[i].revents;
            
            // Skip if no events
            if (revents == 0) continue;
            
            // 1. Handle new connections on listening sockets
            if ((revents & POLLIN) && _fdType[fd] == "listen") {
                acceptConnection(fd);
                continue;
            }
            
            // 2. Handle CGI output ready to read
            if ((revents & POLLIN) && _fdType[fd] == "CGI") {
                std::map<int, Connection>::iterator it = _connections.find(_cgifds[fd]);
                if (it != _connections.end()) {
                    it->second.readCGIOutput();
                    
                    // If CGI reading is complete, prepare to write response
                    if (it->second.cgiDone()) {
                        int conn_fd = it->first;
                        // Find the connection's pollfd and switch to POLLOUT
                        for (size_t j = 0; j < _pollfds.size(); j++) {
                            if (_pollfds[j].fd == conn_fd) {
                                _pollfds[j].events = POLLOUT;
                                _pollfds[j].revents = 0;
                                CHOROUK && std::cout << "CGI done, switching to POLLOUT for fd " << conn_fd << std::endl;
                                break;
                            }
                        }
                    }
                }
                continue;
            }
            
            // 3. Handle client request reading
            if ((revents & POLLIN) && _fdType[fd] == "connection") {
                std::map<int, Connection>::iterator it = _connections.find(fd);
                if (it == _connections.end()) continue;
                
                if (!it->second.readRequest()) {
                    CHOROUK && std::cout << "Read failed or connection closed, fd: " << fd << std::endl;
                    closeConnection(it, i);
                    continue;
                }
                
                // Request is complete - decide next action
                if (it->second.isDone()) {
                    if (it->second.isCGI() && !it->second.cgiDone()) {
                        // Start CGI process
                        int cgifd = it->second.getCgiFd();
                        if (cgifd != -1) {
                            addPollFd(cgifd, POLLIN, "CGI");
                            _cgifds[cgifd] = fd;
                            CHOROUK && std::cout << "Added CGI fd " << cgifd << " for connection " << fd << std::endl;
                        } else {
                            // CGI failed to start, switch to write error
                            _pollfds[i].events = POLLOUT;
                            _pollfds[i].revents = 0;
                        }
                    } else {
                        // Regular request or error - switch to write mode
                        _pollfds[i].events = POLLOUT;
                        _pollfds[i].revents = 0;
                        CHOROUK && std::cout << "Request done, switching to POLLOUT for fd " << fd << std::endl;
                    }
                }
                continue;
            }
            
            // 4. Handle response writing
            if ((revents & POLLOUT) && _fdType[fd] == "connection") {
                std::map<int, Connection>::iterator it = _connections.find(fd);
                if (it == _connections.end()) continue;
                
                // Don't write if still waiting for CGI
                if (it->second.isCGI() && !it->second.cgiDone()) {
                    CHOROUK && std::cout << "Still waiting for CGI on fd " << fd << std::endl;
                    continue;
                }
                
                CHOROUK && std::cout << "Writing response on fd " << fd << std::endl;
                
                if (!it->second.writeResponse()) {
                    MOHAMED && std::cout << "Write failed on fd " << fd << std::endl;
                    closeConnection(it, i);
                    continue;
                }
                
                // Check if response is complete
                if (it->second.isResponseDone()) {
                    CHOROUK && std::cout << "Response complete, closing fd " << fd << std::endl;
                    closeConnection(it, i);
                }
                continue;
            }
            
            // Handle error events
            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                MOHAMED && std::cout << "Error on fd " << fd << ", revents: " << revents << std::endl;
                if (_fdType[fd] == "connection") {
                    std::map<int, Connection>::iterator it = _connections.find(fd);
                    if (it != _connections.end()) {
                        closeConnection(it, i);
                    }
                } else if (_fdType[fd] == "CGI") {
                    // CGI error - close CGI and mark connection for error response
                    close(fd);
                    int conn_fd = _cgifds[fd];
                    _cgifds.erase(fd);
                    _fdType.erase(fd);
                    _pollfds.erase(_pollfds.begin() + i);
                    
                    // Mark connection for error response
                    std::map<int, Connection>::iterator it = _connections.find(conn_fd);
                    if (it != _connections.end()) {
                        it->second.getRequest().setStatusCode(500);
                    }
                }
            }
        }
    }
    
    cleanUp();
    sleep(1);
    std::cout << M"Server closed.\n";
}

void WebServ::closeConnection(std::map<int, Connection>::iterator it, int poll_index) {
    int fd = it->first;
    
    CHOROUK && std::cout << "Closing connection fd " << fd << std::endl;
    
    // Close CGI fd if exists
    if (it->second.isCGI()) {
        int cgifd = it->second.getCgiFd();
        if (cgifd != -1) {
            CHOROUK && std::cout << "Closing CGI fd " << cgifd << std::endl;
            close(cgifd);
            _fdType.erase(cgifd);
            _cgifds.erase(cgifd);
            
            // Remove CGI fd from pollfds
            for (int k = (int)_pollfds.size() - 1; k >= 0; k--) {
                if (_pollfds[k].fd == cgifd) {
                    _pollfds.erase(_pollfds.begin() + k);
                    if (k < poll_index) {
                        poll_index--; // Adjust index
                    }
                    break;
                }
            }
        }
    }
    
    // Close connection fd
    close(fd);
    _connections.erase(it);
    _fdType.erase(fd);
    _pollfds.erase(_pollfds.begin() + poll_index);
}

// Add this to WebServ.hpp:
// void closeConnection(std::map<int, Connection>::iterator it, int poll_index);

bool WebServ::acceptConnection(int fd){
    try{
        CHOROUK && std::cout << "new connection fd " << fd << " " << _server.getRoot() << std::endl; 
        // Connection connection = new Connection(fd, _server);
        Connection connection = Connection(fd, _server, _listenFds[fd].first, _listenFds[fd].second);
        // Connection connection = new Connection(fd, _server, _listenFds[fd].first, _listenFds[fd].second);
        int connection_fd = connection.getFd();
        _connections.insert(std::make_pair(connection_fd, connection));
        CHOROUK && std::cout << "------- accept fd = " << connection_fd << std::endl;
        // struct pollfd client_pfd;
        // client_pfd.fd = connection_fd;
        // client_pfd.events = POLLIN;
        // client_pfd.revents = 0;
        // _pollfds.push_back(client_pfd);
        _cleanRead = false;
        addPollFd(connection_fd, POLLIN|POLLOUT, "connection"); // to change later by macro
    }
    catch (const std::exception& e){
        std::cerr << R"Error: " << e.what() << B"\n";
    }
    return (true);
}

void WebServ::cleanUp(){
    for (int i = (int)_pollfds.size() - 1; i >= 0 ; i--){
        close(_pollfds[i].fd);
        _pollfds.erase(_pollfds.begin() + i);
    }
    _pollfds.clear();
}

void WebServ::checkTimeout(){
    // std::cout << "-*-*-* > timeout check!!\n";
    time_t now = time(NULL);
    for (int i = 0; i < (int)_pollfds.size(); i++){
        int fd = _pollfds[i].fd;
        // std::cout << "-*-*-* > fd = " << fd << "\n";
        if (_fdType[fd] == "listen") continue;
        // if (_fdType[fd] == "CGI"){
        //     fd = _cgiFds[fd];
        //     // std::map<int, int>::iterator it = _cgiFd.find(fd);
        //     // fd = it->
        // }
        std::map<int, Connection>::iterator it = _connections.find(fd);
        if (it != _connections.end() && now - it->second.getTime() > 60){
            std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
            if (it->second.isCGI()){
                int cgifd = it->second.getCgiFd();
                close(cgifd);
                for (int k = 0; k < (int)_pollfds.size(); k++){
                    int fd_cgi = _pollfds[k].fd;
                    if (fd_cgi == cgifd){
                        _pollfds.erase(_pollfds.begin() + k);
                        break;
                    }
                }
                _fdType.erase(cgifd);
            //     std::vector<struct pollfd>::iterator it = _pollfds.find(cgifd);
            //     _pollfds.erase(it);
            }
            // delete it->second;
            close(fd);
            _connections.erase(it);
            _fdType.erase(fd);
            _pollfds.erase(_pollfds.begin() + i);
            i--;
            
        }
    }
}
