#include "WebServ.hpp"

bool WebServ::_runServer = true;

WebServ::~WebServ()
{
    for (int i = (int)_pollfds.size() - 1; i >= 0; i--)
        close(_pollfds[i].fd);
    _pollfds.clear();
}

WebServ::WebServ(ServerConf& server): _server(server) {}

bool WebServ::startServer(ServerConf& server)
{
    WebServ webserv(server);
    std::vector<std::pair<std::string, std::string> > listens = server.getListen();
    std::vector<std::pair<std::string, std::string> >::iterator it;
    for (it = listens.begin(); it != listens.end(); it++)
    {
        try
        {
            int fd = Socket::StartSocket(it->first, it->second);
            webserv.addPollFd(fd, POLLIN, "listen");
            std::cout << "Listening on http" << M" " << it->first << ":" << it->second << B"" << std::endl;
            webserv._sockets[fd] = *it;
        }
        catch (const std::exception& e)
        {
            std::cerr << R"Error: " << e.what() << B"\n";
        }
    }
    if (webserv._pollfds.size() == 0)
    {
        std::cerr << R"All listens failed. Server closed.\n";
        exit(1);
    }
    std::cout << B"Document root is " << M" " << server.getRoot() << std::endl;
    std::cout << B"Press Ctrl-C to quit." << B"\n";
    webserv.pollLoop();
    return (true);
}

void WebServ::addPollFd(int fd, short event, const std::string& type)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = event;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
    _fdType[fd] = type;

}

void WebServ::pollLoop()
{
    while (_runServer)
    {
        int n = poll(_pollfds.data(), _pollfds.size(), 1000);
        if (!checkStatus(n))
            break;
        std::vector<int> to_remove;
        for (int i = (int)_pollfds.size() - 1; i >= 0; i--){
            int fd = _pollfds[i].fd;
            short revents = _pollfds[i].revents;
            std::string type = _fdType[fd];
            // std::cout << "-*-*-*-*-* > plllfd size = " << _pollfds.size() << " fd = " << fd << " fd_type = " << type << std::endl;
            if (revents & POLLIN && type == "listen")
                    acceptConnection(fd);
            if (revents & POLLIN && type == "CGI"){
                    std::map<int, Connection>::iterator it = _connections.find(_cgifds[fd]);
                    it->second.readCGIOutput();
            }
            else if ((revents & POLLIN)){
                if (type == "connection"){
                    std::map<int, Connection>::iterator it = _connections.find(fd);
                    if (it == _connections.end()) continue;
                    // it->second.readRequest();
                    if (!it->second.readRequest()){
                        _pollfds[i].events = POLLOUT;
                        continue;
                    }
                    
                    if (it->second.isDone()){
                        if (it->second.isCGI()){
                            int cgifd = it->second.getCgiFd();
                            addPollFd(cgifd, POLLIN, "CGI");
                            _cgifds[cgifd] = fd;
                        }
                    }
                    else {
                        CHOROUK && std::cout << "----------- READ NOT DONE ---------------\n";
                    }
                }
            }
            else if (revents & POLLOUT){
                CHOROUK && std::cout << "----------- INSIDE POLLOUT ---------------\n";
                std::map<int, Connection>::iterator it = _connections.find(fd);
                if (it == _connections.end()) continue;
                if (it->second.isCGI() && !it->second.cgiDone()) continue;
                CHOROUK && std::cout << "----------- INSIDE WRITE ---------------\n";
                it->second.writeResponse();
                CHOROUK && std::cout << "----------- AFTER WRITE ---------------\n";
                if (it->second.isResponseDone()) {
                    CHOROUK && std::cout << "----------- WRITE DONE ---------------\n";
                    close(fd);
                    to_remove.push_back(fd);
                    // delete it->second;
                    if (it->second.isCGI()){
                        int n = it->second.getCgiFd();
                        close(n);
                        to_remove.push_back(n);
                    }
                    _connections.erase(it);
                }
                else {
                    CHOROUK && std::cout << "----------- WRITE NOT DONE ---------------\n";
                }

            }
        }
        for (unsigned int j = 0; j < to_remove.size(); j++) {
            int fd = to_remove[j];
            _fdType.erase(fd);
            for (unsigned int k = 0; k < _pollfds.size(); k++) {
                if (_pollfds[k].fd == fd) {
                    _pollfds.erase(_pollfds.begin() + k);
                    break;
                }
            }
        }
    }
    cleanUp();
    sleep(3);
    std::cout << M"Server closed.\n";
}

bool WebServ::acceptConnection(int fd){
    try{
        CHOROUK && std::cout << "new connection fd " << fd << " " << _server.getRoot() << std::endl; 
        Connection connection = Connection(fd, _server, _sockets[fd].first, _sockets[fd].second);
        int connection_fd = connection.getFd();
        _connections.insert(std::make_pair(connection_fd, connection));
        CHOROUK && std::cout << "------- accept fd = " << connection_fd << std::endl;
        // _cleanRead = false;
        addPollFd(connection_fd, POLLIN|POLLOUT, "connection");
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

bool WebServ::checkStatus(int n) {
    if (n == -1)
    {
        if ((errno == EINTR) && !_runServer)
            std::cout << "Server is shutting down...\n";
        else
            std::cerr << "Error: Poll failed\n";
        return (false);
    }

    time_t now = time(NULL);
    // for (int i = 0; i < (int)_pollfds.size(); i++) {
    //     int fd = _pollfds[i].fd;
    //     if (_fdType[fd] == "listen") continue;

    //     std::map<int, Connection>::iterator it = _connections.find(fd);
    //     if (it != _connections.end() && now - it->second.getTime() > TIMEOUT) {
    //         std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
    //         if (it->second.isCGI()) {
    //             int cgifd = it->second.getCgiFd();
    //             close(cgifd);

    //             // Remove CGI fd from pollfds
    //             for (unsigned int k = 0; k < _pollfds.size(); k++) {
    //                 if (_pollfds[k].fd == cgifd) {
    //                     _pollfds.erase(_pollfds.begin() + k);
    //                     break;  // Once removed, break the loop
    //                 }
    //             }
    //             _fdType.erase(cgifd);
    //         }
    //         close(fd);
    //         _connections.erase(it);
    //         _fdType.erase(fd);
    //         _pollfds.erase(_pollfds.begin() + i);
    //         i--; // Adjust index after removal
    //     }
    // }
    for (int i = (int)_pollfds.size() - 1; i >= 0; i--)
    {
        int fd = _pollfds[i].fd;
        if (_fdType[fd] == "listen" || _fdType[fd] == "CGI") continue;

        std::map<int, Connection>::iterator it = _connections.find(fd);
        if (it != _connections.end() && now - it->second.getTime() > TIMEOUT) {
            std::cout << "Closing connection fd " << fd << " due to timeout." << std::endl;
            if (it->second.isCGI()) {
                int cgifd = it->second.getCgiFd();
                close(cgifd);

                // Remove CGI fd from pollfds
                int k;
                for (k = 0; k < (int)_pollfds.size(); k++) {
                    if (_pollfds[k].fd == cgifd) {
                        _pollfds.erase(_pollfds.begin() + k);
                        break;  // Once removed, break the loop
                    }
                }
                _fdType.erase(cgifd);
                // if (k <= i) i--;
            }
            close(fd);
            _connections.erase(it);
            _fdType.erase(fd);
            _pollfds.erase(_pollfds.begin() + i);
            i--; // Adjust index after removal
        }
    }
    return (true);
}