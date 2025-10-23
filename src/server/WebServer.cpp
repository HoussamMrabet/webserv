#include "WebServ.hpp"

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
        int fd = createListenSocket(addresses[i].first, addresses[i].second);
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
                    // std::cout << "new connection\n";
                    acceptClient(fd);
                }
                break;
                
            case Connection::CLIENT:
                if (fds[i].revents & POLLIN && c->state == Connection::READING) {
                    // std::cout << "new request\n";
                    if (!readRequest(c)) {
                        closeConn(fd);
                    }
                }
                if (fds[i].revents & POLLOUT && c->state == Connection::WRITING) {
                    // std::cout << "new response\n";
                    if (!writeResponse(c)) {
                        closeConn(fd);
                    }
                }
                break;
                
            case Connection::CGI:
                if (fds[i].revents & POLLIN) {
                    // std::cout << "new cgi\n";
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
    // std::cout << "request received:\n" << buf << std::endl;
    
    if (n < 0) {
        return (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    }
    else if (n > 0){
        c->read_buf.append(buf, n);
        c->req->parseRequest(c->read_buf);
        c->read_buf.clear();
    }
    else if (n == 0){
        c->req->parseRequest();
    }
    if (c->req->isDone()){
        // std::cout << "request is done\n";
        c->req->processRequest();

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
    // std::cout << "response:\n" << c->write_buf << std::endl;
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
        
        execCGI(*c->req, config);
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

int WebServer::createListenSocket(const std::string& host, const std::string& port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error: Failed to create socket" << std::endl;
        return -1;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error: setsockopt SO_REUSEADDR failed" << std::endl;
        close(sockfd);
        return -1;
    }
    
    // Set non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "Error: Failed to set non-blocking" << std::endl;
        close(sockfd);
        return -1;
    }
    
    // Setup address structure
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    
    // Parse host
    if (host.empty() || host == "0.0.0.0" || host == "*") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
            std::cerr << "Error: Invalid host address: " << host << std::endl;
            close(sockfd);
            return -1;
        }
    }
    
    // Parse port
    int port_num = 0;
    std::istringstream iss(port);
    iss >> port_num;
    if (iss.fail() || port_num < 0 || port_num > 65535) {
        std::cerr << "Error: Invalid port: " << port << std::endl;
        close(sockfd);
        return -1;
    }
    addr.sin_port = htons(port_num);
    
    // Bind
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: bind failed on " << host << ":" << port 
                  << " - " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    
    // Listen
    if (listen(sockfd, 128) < 0) {
        std::cerr << "Error: listen failed - " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

void WebServer::execCGI(const Request& req, ServerConf& config) {
    // Setup environment variables
    std::map<std::string, std::string> env;
    
    // Required CGI variables
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_SOFTWARE"] = "webserv/1.0";
    env["REQUEST_METHOD"] = req.getMethod();
    env["SCRIPT_NAME"] = req.getUri();
    env["PATH_INFO"] = req.getUri();
    // env["SCRIPT_NAME"] = req.getPath();
    // env["PATH_INFO"] = req.getPath();
    env["QUERY_STRING"] = req.getUriQueries();
    env["CONTENT_TYPE"] = req.getHeader("Content-Type");
    env["REDIRECT_STATUS"] = "200";
    
    // Content length
    std::ostringstream oss;
    oss << req.getBody().size();
    env["CONTENT_LENGTH"] = oss.str();
    
    std::vector<std::pair<std::string, std::string> >listen = config.getListen();
    std::vector<std::pair<std::string, std::string> >::iterator itt = listen.begin();
    // Server info
    oss.str("");
    oss << itt->second;
    env["SERVER_PORT"] = oss.str();
    std::set<std::string> name = config.getServerNames();
    std::set<std::string>::iterator it = name.begin();
    env["SERVER_NAME"] = *it;//////
    // env["SERVER_NAME"] = config.getServerName();
    
    // Remote info
    env["REMOTE_ADDR"] = "127.0.0.1";
    
    // HTTP headers as HTTP_*
    const std::map<std::string, std::string>& headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        std::string key = "HTTP_" + it->first;
        // Convert to uppercase and replace - with _
        for (size_t i = 5; i < key.length(); i++) {
            if (key[i] == '-')
                key[i] = '_';
            else if (key[i] >= 'a' && key[i] <= 'z')
                key[i] = key[i] - 32;
        }
        env[key] = it->second;
    }
    
    // Get script path
    // std::string scriptPath = config.getRoot() + req.getPath();
    std::string scriptPath = req.getFullPath();
    env["SCRIPT_FILENAME"] = scriptPath;
    

    std::map<std::string, LocationConf> locations = config.getLocations();
    LocationConf conf = locations[req.getLocation()];
    std::map<std::string, std::string> cgis = conf.getCgi();

    // Get CGI interpreter
    std::string interpreter = cgis[req.getCgiType()];
    
    // Convert environment to char**
    char** envp = new char*[env.size() + 1];
    size_t i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env.begin();
         it != env.end(); ++it, ++i) {
        std::string envStr = it->first + "=" + it->second;
        envp[i] = new char[envStr.length() + 1];
        std::strcpy(envp[i], envStr.c_str());
    }
    envp[i] = NULL;
    
    // Change directory to script directory
    size_t lastSlash = scriptPath.rfind('/');
    if (lastSlash != std::string::npos) {
        std::string dir = scriptPath.substr(0, lastSlash);
        if (chdir(dir.c_str()) < 0) {
            std::cerr << "Error: chdir failed" << std::endl;
        }
    }
    
    // Execute CGI
    char* argv[3];
    if (!interpreter.empty()) {
        argv[0] = const_cast<char*>(interpreter.c_str());
        argv[1] = const_cast<char*>(scriptPath.c_str());
        argv[2] = NULL;
        execve(interpreter.c_str(), argv, envp);
    } else {
        argv[0] = const_cast<char*>(scriptPath.c_str());
        argv[1] = NULL;
        execve(scriptPath.c_str(), argv, envp);
    }
    
    // If we reach here, exec failed
    std::cerr << "Error: execve failed - " << strerror(errno) << std::endl;
    
    // Cleanup
    for (size_t j = 0; j < env.size(); j++) {
        delete[] envp[j];
    }
    delete[] envp;
}
