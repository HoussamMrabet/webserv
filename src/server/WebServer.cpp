#include "WebServ.hpp"

bool WebServ::_runServer = true;

// void WebServ::signalHandler(int n) {
//     std::cout << C"Server closing...\n";
//     _runServer = false;
//     signal(n, SIG_IGN); // ignore
//     // clean up
//     // exit(0); // then close
// }

WebServ::WebServ(){/*init default data*/}

WebServ::~WebServ(){
    // loop on pollfds and close all
    // delete connection pointers  
    for (int i = 0; i < (int)_pollfds.size(); i++){
        // int fd = _pollfds[i].fd;

        // std::map<int, Connection>::iterator it = _connections.find(fd);
        // if (it != _connections.end() && now - it->second.getTime() > 60){
        //     std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
            // delete it->second;
            close(_pollfds[i].fd);
            // _connections.erase(it);
            // _fdType.erase(fd);
            _pollfds.erase(_pollfds.begin() + i);
            i--;
            
        // }
    }
    _pollfds.clear();
}

WebServ::WebServ(ServerConf& server): _server(server) {/*init data*/}

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
    while (_runServer){
        CHOROUK && std::cout << "----------- IN POLLOOP ---------------\n";
        int n = poll(_pollfds.data(), _pollfds.size(), 1000);  // 1-second poll timeout to check timeouts regularly
        CHOROUK && std::cout << "pollfs size = " << _pollfds.size() << std::endl;
        if (n == -1) {
            if (errno == EINTR) {
                // Poll interrupted by signal
                if (!_runServer) {
                    std::cout << "Server shutting down...\n";
                    break;  // Exit the loop gracefully
                }
                else {
                    std::cout << "Server still runing...\n";
                    continue;  // Otherwise just continue polling
                }
            }
            else {
                std::cerr << R"Error: Poll failed" << B"\n";
                cleanUp();
                // clean all before exit
            //     perror("Poll failed");
                break; // Fatal error, exit loop
            }
        }
        // if (n == -1){
        //     perror("Poll failed");
        //     break; // Only break on fatal poll error
        // }
        checkTimeout();
        // if (n == 0) continue;  // no events, continue loop
        
        // Process events - iterate backwards to handle erasing safely
        for (int i = (int)_pollfds.size() - 1; i >= 0; i--){
            // sleep (1);
            CHOROUK && std::cout << "----------- INSIDE POLLOOP ---------------\n";
            CHOROUK && std::cout << "------- pollfd_size = " << (int)_pollfds.size() << std::endl;
            CHOROUK && std::cout << "i = " << i << "\n";
            int fd = _pollfds[i].fd;
            CHOROUK && std::cout << "fd = " << fd << std::endl;
            CHOROUK && std::cout << "fdType = " << _fdType[fd] << std::endl;
            
                // CHOROUK && std::cout << "----------- INSIDE POLLIN ---------------\n";
            if (_pollfds[i].revents & POLLIN && _fdType[fd] == "listen"){
                    CHOROUK && std::cout << "----------- INSIDE LISTEN ---------------\n";
                    acceptConnection(fd); // Always try to accept new connections
            }
            if (_pollfds[i].revents & POLLIN && _fdType[fd] == "CGI"){
                    CHOROUK && std::cout << "----------- INSIDE CGI read ---------------\n";
                    // keep reading from cgi fd and write to corresponding connection
                    std::map<int, Connection>::iterator it = _connections.find(_cgifds[fd]);
                    // it->second.writeResponse();
                    it->second.readCGIOutput();
                    // continue;
                    // it->second. 
            }
            else if ((_pollfds[i].revents & POLLIN) || !_cleanRead){
                if (_fdType[fd] == "connection"){
                    CHOROUK && std::cout << "----------- INSIDE CONNECTION ---------------\n";
                    std::map<int, Connection>::iterator it = _connections.find(fd);
                    if (it == _connections.end()) continue; // Safety check - connection not found
                    CHOROUK && std::cout << "----------- INSIDE READ ---------------\n";
                    if (!it->second.readRequest()){
                        CHOROUK && std::cout << "----------- READ FAIL ---------------\n";
                        // Connection error - clean up and continue
                        close(fd);
                        // delete it->second;
                        _connections.erase(it);
                        _fdType.erase(fd);
                        _pollfds.erase(_pollfds.begin() + i);
                        continue;
                    }
                    
                    if (it->second.isDone()){
                        CHOROUK && std::cout << "----------- READ DONE ---------------\n";
                        _cleanRead = true;
                        if (it->second.isCGI()/* && it->second.execDone()*/){
                            std::map<int, int>::iterator itt = _cgifds.begin();
                            while ((itt != _cgifds.end()) && (itt->first != fd)) itt++;
                            if (itt == _cgifds.end()){
                                int cgifd = it->second.getCgiFd();
                                addPollFd(cgifd, POLLIN, "CGI");
                                _cgifds[cgifd] = fd;
                                CHOROUK && std::cout << "*-*-*-*-*-*-*-*->> Request is CGI!! \n" << std::endl;
                                // std::cout << "cgi fd is " << cgifd << " connection fd is " << _cgifds[cgifd] << std::endl;
                            }
                        
                        //     //execute cgi and get cgi fd
                        //     // add cgi fd to pollfds if not exist!!!
                        }
                        // if (it->second.getCgiFd() != -1){
                        //     // add cgi fd to pollfds if not exist!!!
                        //     // execute cgi and get cgi fd
                        // }
                        // // Request complete, switch to write mode
                        // // _pollfds[i].events = POLLOUT;
                        // // _pollfds[i].revents = 0;
                        // // it->second.printRequest(); // to remove!
                    }
                    else {
                        CHOROUK && std::cout << "----------- READ NOT DONE ---------------\n";
                        // Request complete, switch to write mode
                        // _pollfds[i].events = POLLIN;
                        // _pollfds[i].revents = 0;
                        // it->second.printRequest(); // to remove!
                    }
                }
            }
            else if (_pollfds[i].revents & POLLOUT){
                CHOROUK && std::cout << "----------- INSIDE POLLOUT ---------------\n";
                std::map<int, Connection>::iterator it = _connections.find(fd);
                if (it == _connections.end()) continue; // Safety check
                if (it->second.isCGI() && !it->second.cgiDone()) continue;
                CHOROUK && std::cout << "----------- INSIDE WRITE ---------------\n";
                it->second.writeResponse();
                CHOROUK && std::cout << "----------- AFTER WRITE ---------------\n";
                if (it->second.isResponseDone()) {
                    CHOROUK && std::cout << "----------- WRITE DONE ---------------\n";
                    // _pollfds[i].events = POLLIN;
                    // _pollfds[i].revents = 0;
                        close(fd);
                        // delete it->second;
                        if (it->second.isCGI()){
                            int n = it->second.getCgiFd();
                            close(n);
                            _fdType.erase(n);
                            int k = 0;
                            while (k < (int)_pollfds.size()){
                                if (_pollfds[k].fd == n){
                                    _pollfds.erase(_pollfds.begin() + k);
                                    break;
                                }
                                k++;
                            }
                        }
                        _connections.erase(it);

                        // _cgiFd[]
                        _fdType.erase(fd);
                        _pollfds.erase(_pollfds.begin() + i);
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
                    // _pollfds[i].revents = 0;
                }

            }
            // else if (_pollfds[i].revents & POLLIN && _fdType[fd] == "cgi_output") {
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
    sleep(1);
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
}

void WebServ::checkTimeout(){
    // std::cout << "-*-*-* > timeout check!!\n";
    time_t now = time(NULL);
    for (int i = 0; i < (int)_pollfds.size(); i++){
        int fd = _pollfds[i].fd;
        // std::cout << "-*-*-* > fd = " << fd << "\n";
        if (_fdType[fd] == "listen") continue;
        std::map<int, Connection>::iterator it = _connections.find(fd);
        if (it != _connections.end() && now - it->second.getTime() > 60){
            std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
            // delete it->second;
            close(fd);
            _connections.erase(it);
            _fdType.erase(fd);
            _pollfds.erase(_pollfds.begin() + i);
            i--;
            
        }
    }
    
    // if (_pid != -1) {// Check if the CGI process is finished
    //     int status;
    //     pid_t result = waitpid(_pid, &status, WNOHANG);
    
    //     if (result == -1) {
    //         perror("waitpid failed");
    //     } else if (result == 0) {
    //         // Process is still running
    //     } else {
    //         // Process has finished
    //         if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
    //             std::cout << "CGI process completed successfully." << std::endl;
    //             // Mark the CGI process as done or ready to read
    //             addPollFd(_fd, POLLIN, "cgi_output");  // Ready to read CGI output
    //         } else {
    //             std::cerr << "CGI process failed with status: " << WEXITSTATUS(status) << std::endl;
    //             // Handle the failure (e.g., cleaning up, reporting errors)
    //         }
    //         _pid = -1;  // Clear the PID once the process is handled
    //     }
    // }
}

// void WebServ::closeConnection(int fd){


// }

// void Connection::readCGIOutput() {_cgi}