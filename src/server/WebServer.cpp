#include "WebServ.hpp"

bool WebServ::_runServer = true;

// WebServ::WebServ(){}

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
    // _cleanRead = false;
    while (_runServer){
        int n = poll(_pollfds.data(), _pollfds.size(), 1000);
        // if (n == 0) continue;
        CHOROUK && std::cout << "pollfs size = " << _pollfds.size() << std::endl;
        if (n == -1) {
            if (errno == EINTR) {
                if (!_runServer) {
                    std::cout << "Server shutting down...\n";
                    break;
                }
                else {
                    std::cout << "Server still runing...\n";
                    continue;
                }
            }
            else {
                std::cerr << R"Error: Poll failed" << B"\n";
                std::cerr << "ERROR NUM: " << errno << " pollfds: " << _pollfds.data() << " pollfds size: " << _pollfds.size() << std::endl; 
                // cleanUp();
                break;
            }
        }

        checkTimeout();
        for (int i = (int)_pollfds.size() - 1; i >= 0; i--){
            int fd = _pollfds[i].fd;
            short revents = _pollfds[i].revents;
            std::string type = _fdType[fd];

        if (revents & (POLLERR | POLLNVAL)) {
            if (type != "listen") {
                closeConnection(fd);
            }
            continue;
        }
            // std::cout << "-*-*-*-*-* > plllfd size = " << _pollfds.size() << " fd = " << fd << " fd_type = " << _fdType[fd] << std::endl;
            if (revents & POLLIN && type == "listen")
                    acceptConnection(fd);
            else if (revents & POLLIN && type == "CGI"){
                    std::map<int, Connection>::iterator it = _connections.find(_cgifds[fd]);
                    it->second.readCGIOutput();
            }
            else if ((revents & POLLIN)/* || !_cleanRead*/){
                if (type == "connection"){
                    std::map<int, Connection>::iterator it = _connections.find(fd);
                    if (it == _connections.end()) continue;
                    if (!it->second.readRequest()){ // remove this check!! read never returns false
                        // close(fd);
                        // _connections.erase(it);
                        // _fdType.erase(fd);
                        // _pollfds.erase(_pollfds.begin() + i);
                        closeConnection(fd);
                        continue;
                    }
                    
                    if (it->second.isDone()){
                        // std::cout << "-*-*-*-*-* > I m In !!!!" << std::endl;
                        if (it->second.isCGI() && !it->second.cgiDone()/* && !_cleanRead*/){
                        // if (it->second.isCGI()){
                            int cgifd = it->second.getCgiFd();
                            // std::map<int, int>::iterator itt = _cgifds.find(cgifd);
                            // if (itt != _cgifds.end()) continue;
                            // // while ((itt != _cgifds.end()) && (itt->first != cgifd)) itt++;
                            // if (itt != _cgifds.end()) std::cout << R"already exist!!\n";
                                // std::cout << "-*-*-*-*-* > new cgi = " << cgifd << " connection fd = " << fd << std::endl;
                                // std::cout << "-*-*-*-*-* > plllfd size = " << _pollfds.size() << " cgifd = " << cgifd << std::endl;
                                addPollFd(cgifd, POLLIN, "CGI");
                                _cgifds[cgifd] = fd;
                                CHOROUK && std::cout << "*-*-*-*-*-*-*-*->> Request is CGI!! \n" << std::endl;
                                // std::cout << "cgi fd is " << cgifd << " connection fd is " << _cgifds[cgifd] << std::endl;
                            // }
                        
                        //     //execute cgi and get cgi fd
                        //     // add cgi fd to pollfds if not exist!!!
                        }
                        else if (it->second.isCGI() && it->second.cgiDone()){
                            int cgi_fd = it->second.getCgiFd();
                            removePollFd(cgi_fd);
                            _cgifds.erase(cgi_fd);
                            _fdType.erase(cgi_fd);
                            _connections.erase(cgi_fd);
                        }
                        // _cleanRead = true;
                        // if (it->second.getCgiFd() != -1){
                        //     // add cgi fd to pollfds if not exist!!!
                        //     // execute cgi and get cgi fd
                        // }
                        // // Request complete, switch to write mode
                        // // _pollfds[i].events = POLLOUT;
                        // // revents = 0;
                        // // it->second.printRequest(); // to remove!
                    }
                    else {
                        CHOROUK && std::cout << "----------- READ NOT DONE ---------------\n";
                        // Request complete, switch to write mode
                        // _pollfds[i].events = POLLIN;
                        // revents = 0;
                        // it->second.printRequest(); // to remove!
                    }
                }
            }
            else if (revents & POLLOUT){
                CHOROUK && std::cout << "----------- INSIDE POLLOUT ---------------\n";
                std::map<int, Connection>::iterator it = _connections.find(fd);
                if (it == _connections.end()) continue; // Safety check
                if (it->second.isCGI() && !it->second.cgiDone()) continue;
                CHOROUK && std::cout << "----------- INSIDE WRITE ---------------\n";
                it->second.writeResponse();
                CHOROUK && std::cout << "----------- AFTER WRITE ---------------\n";
                if (it->second.isResponseDone()) {
                    CHOROUK && std::cout << "----------- WRITE DONE ---------------\n";
                    closeConnection(fd);
                    // // _pollfds[i].events = POLLIN;
                    // // revents = 0;
                    //     close(fd);
                    //     // delete it->second;
                    //     if (it->second.isCGI()){
                    //         int n = it->second.getCgiFd();
                    //         close(n);
                    //         _fdType.erase(n);
                    //         int k = 0;
                    //         while (k < (int)_pollfds.size()){
                    //             if (_pollfds[k].fd == n){
                    //                 _pollfds.erase(_pollfds.begin() + k);
                    //                 break;
                    //             }
                    //             k++;
                    //         }
                    //     }
                    //     _connections.erase(it);

                    //     // _cgiFd[]
                    //     _fdType.erase(fd);
                    //     _pollfds.erase(_pollfds.begin() + i);
                        // continue;
                    // if (it->second.getConnectionHeader() == "close"){ // check if "close" close connection
                    //     // closeConnection();
                    //     CHOROUK && std::cout << "Close connection header fd " << fd << std::endl;
                        // delete it->second;
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
                    // revents = 0;
                }

            }
            // else if (revents & POLLIN && type == "cgi_output") {
            //     // Handle CGI output read
            //     std::string cgi_output = readCGIOutput();
            //     if (!cgi_output.empty()) {
            //         // Process the CGI output (write it back to the connection, for example)
            //         std::cout << cgi_output << std::endl;
            //     }
            // }
        }
        // After processing all events, continue to next poll cycle
        // The server NEVER stops listening for new connections
    }
    cleanUp();
    sleep(3);
    std::cout << M"Server closed.\n";
}

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
    _connections.clear();

}

void WebServ::checkTimeout(){
    // std::cout << "-*-*-* > timeout check!!\n";
    time_t now = time(NULL);
    for (int i = 0; i < (int)_pollfds.size(); i++){
        int fd = _pollfds[i].fd;
        // std::cout << "-*-*-* > fd = " << fd << "\n";
        if (_fdType[fd] == "listen") continue;
        // if (_fdType[fd] == "CGI") continue;
        // if (_fdType[fd] == "CGI"){
        //     fd = _cgiFds[fd];
        //     // std::map<int, int>::iterator it = _cgiFd.find(fd);
        //     // fd = it->
        // }
        std::map<int, Connection>::iterator it = _connections.find(fd);
        if (it != _connections.end() && now - it->second.getTime() > TIMEOUT){
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

void WebServ::removePollFd(int fd) {
    for (size_t i = 0; i < _pollfds.size(); ++i) {
        if (_pollfds[i].fd == fd) {
            _pollfds.erase(_pollfds.begin() + i);
            break;
        }
    }
}

void WebServ::closeConnection(int fd) {
    std::map<int, Connection>::iterator it = _connections.find(fd);
    if (it == _connections.end()) return;
    
    if (it->second.isCGI()) {
        int cgi_fd = it->second.getCgiFd();
        if (cgi_fd > 0) {
            close(cgi_fd);
            removePollFd(cgi_fd);
            _fdType.erase(cgi_fd);
            _connections.erase(cgi_fd);
        }
    }
    
    close(fd);
    removePollFd(fd);
    _fdType.erase(fd);
    _connections.erase(it);
}