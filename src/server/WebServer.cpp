#include "WebServ.hpp"

bool WebServer::running = true;

WebServer::WebServer(ServerConf& conf) : config(conf) {}

WebServer::~WebServer() {
    for (std::map<int, Connection*>::iterator it = conns.begin(); 
            it != conns.end(); ++it) {
        close(it->first);
        delete it->second;
    }
}

bool WebServer::start() {
    std::vector<std::pair<std::string, std::string> > listens = config.getListen();
    for (size_t i = 0; i < listens.size(); ++i) {
        int fd = startSocket(listens[i].first, listens[i].second);
        if (fd < 0) continue;
        
        addPoll(fd, POLLIN);
        Connection* conn = new Connection(Connection::LISTEN);
        conn->host = listens[i].first;
        conn->port = listens[i].second;
        conn->fd = fd;
        conns[fd] = conn;
        
        std::cout << "Listening on " << listens[i].first 
                    << ":" << listens[i].second << std::endl;
    }
    return !fds.empty();
}

void WebServer::run() {
    while (running) {
        int n = poll(&fds[0], fds.size(), 1000);
        if (n < 0) {
            if (errno == EINTR) continue;
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }
        
        cleanup();
        
        for (int i = static_cast<int>(fds.size()) - 1; i >= 0; i--) {
            if (fds[i].revents == 0) continue;
            
            int fd = fds[i].fd;
            std::map<int, Connection*>::iterator it = conns.find(fd);
            if (it == conns.end()) {
                std::cerr << "DEBUG: fd " << fd << " not in connections map!" << std::endl;
                continue;
            }
            
            Connection* c = it->second;
            c->touch();
            
             // if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            //     std::cout << "DEBUG: Error event on fd " << fd << " type=" << c->type 
            //              << " revents=" << fds[i].revents << std::endl;
            //     if (c->type != Connection::LISTEN) {
            //         closeConn(fd);
            //     }
            //     continue;
            // }
            
            // if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            //     std::cout << "DEBUG: Error event on fd " << fd << " type=" << c->type 
            //              << " revents=" << fds[i].revents << std::endl;
            //     if (c->type != Connection::LISTEN) {
            //         closeConn(fd);
            //     }
            //     continue;
            // }

            // if (fds[i].revents & (POLLERR | POLLNVAL)) {
            //     std::cout << "DEBUG: Error event on fd " << fd << " type=" << c->type 
            //             << " revents=" << fds[i].revents << std::endl;
            //     closeConn(fd);
            //     continue;
            // }

            // if (fds[i].revents & POLLHUP) {
            //     std::cout << "DEBUG: POLLHUP on fd " << fd << " type=" << c->type << std::endl;
            //     if (c->type == Connection::CGI) {
            //         readCGI(c);       // read remaining data
            //         finishCGI(c);     // build and send response
            //     } else {
            //         closeConn(fd);
            //     }
            //     continue;
            // }

            switch (c->type) {
            case Connection::LISTEN:
                if (fds[i].revents & POLLIN) {
                    std::cout << "DEBUG: Accept event on fd " << fd << std::endl;
                    acceptClient(fd);
                }
                break;
                
            case Connection::CLIENT:
                if (fds[i].revents & POLLIN && c->state == Connection::READING) {
                    std::cout << "DEBUG: Read event on client fd " << fd << std::endl;
                    if (!readRequest(c)) {
                        closeConn(fd);
                    }
                }
                if (fds[i].revents & POLLOUT && c->state == Connection::WRITING) {
                    std::cout << "DEBUG: Write event on client fd " << fd << std::endl;
                    if (!writeResponse(c)) {
                        closeConn(fd);
                    }
                }
                break;
                
            case Connection::CGI:
                if (fds[i].revents & POLLIN) {
                    std::cout << "DEBUG: Read event on CGI fd " << fd << std::endl;
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
    std::cout << "DEBUG: Added fd " << fd << " to poll (events=" << events << ")" << std::endl;
}

void WebServer::removePoll(int fd) {
    for (std::vector<struct pollfd>::iterator it = fds.begin(); 
            it != fds.end(); ++it) {
        if (it->fd == fd) {
            std::cout << "DEBUG: Removed fd " << fd << " from poll" << std::endl;
            fds.erase(it);
            break;
        }
    }
}

void WebServer::modifyPoll(int fd, short events) {
    for (size_t i = 0; i < fds.size(); ++i) {
        if (fds[i].fd == fd) {
            std::cout << "DEBUG: Modified fd " << fd << " events: " 
                     << fds[i].events << " -> " << events << std::endl;
            fds[i].events = events;
            break;
        }
    }
}

void WebServer::closeConn(int fd) {
    std::map<int, Connection*>::iterator it = conns.find(fd);
    if (it == conns.end()) return;
    
    Connection* c = it->second;
    std::cout << "DEBUG: Closing fd " << fd << " (type=" << c->type << ")" << std::endl;
    
    // Close related CGI if client closes
    if (c->type == Connection::CLIENT) {
        for (std::map<int, Connection*>::iterator cgi_it = conns.begin();
                cgi_it != conns.end(); ++cgi_it) {
            if (cgi_it->second->type == Connection::CGI && 
                cgi_it->second->parent_fd == fd) {
                std::cout << "DEBUG: Closing related CGI fd " << cgi_it->first << std::endl;
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
        std::cout << "DEBUG: Timeout - closing fd " << expired[i] << std::endl;
        closeConn(expired[i]);
    }
}

void WebServer::acceptClient(int listen_fd) {
    int fd = accept(listen_fd, NULL, NULL);
    if (fd < 0) {
        std::cerr << "DEBUG: Accept failed: " << strerror(errno) << std::endl;
        return;
    }
    
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    
    Connection* c = new Connection(Connection::CLIENT);
    c->fd = fd;
    c->host = conns[listen_fd]->host;
    c->port = conns[listen_fd]->port;
    c->req = new Request();
    
    conns[fd] = c;
    addPoll(fd, POLLIN);
    
    std::cout << "DEBUG: Accepted new client on fd " << fd << std::endl;
}

bool WebServer::readRequest(Connection* c) {
    char buf[4096];
    ssize_t n = read(c->fd, buf, sizeof(buf));
    
    std::cout << "DEBUG: Read " << n << " bytes from fd " << c->fd << std::endl;
    
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;
        }
        std::cerr << "DEBUG: Read error: " << strerror(errno) << std::endl;
        return false;
    }
    else if (n > 0){
        c->read_buf.append(buf, n);
        c->req->parseRequest(c->read_buf);
        c->read_buf.clear();
    }
    else if (n == 0){
        std::cout << "DEBUG: Client closed connection (EOF)" << std::endl;
        c->req->parseRequest();
    }
    
    if (c->req->isDone()){
        std::cout << "DEBUG: Request parsing complete on fd " << c->fd << std::endl;
        c->req->processRequest();
        c->state = Connection::PROCESSING;
        modifyPoll(c->fd, 0); // Remove POLLIN
        return processRequest(c);
    }

    return true;
}

bool WebServer::processRequest(Connection* c) {
    if (c->req->isCGI()) {
        std::cout << "DEBUG: Request is CGI, starting CGI process" << std::endl;
        return startCGI(c);
    }
    
    std::cout << "DEBUG: Processing regular HTTP request" << std::endl;
    c->resp = new Response();
    c->resp->handle(*c->req, config);
    c->write_buf = c->resp->build();
    c->write_pos = 0;
    c->state = Connection::WRITING;
    modifyPoll(c->fd, POLLOUT);
    return true;
}

bool WebServer::startCGI(Connection* c) {
    int pipes_out[2];  // CGI stdout to parent
    int fd_in = c->req->getCgiFdRead();   // Parent to CGI stdin
    
    if (pipe(pipes_out) < 0) {
        std::cerr << "DEBUG: pipe(out) failed: " << strerror(errno) << std::endl;
        close(fd_in);
        return false;
    }
    
    // if (pipe(pipes_in) < 0) {
    //     std::cerr << "DEBUG: pipe(in) failed: " << strerror(errno) << std::endl;
    //     close(pipes_out[0]);
    //     close(pipes_out[1]);
    //     return false;
    // }
    
    std::cout << "DEBUG: Created fdin: stdin=" << fd_in 
             << "] stdout=[" << pipes_out[0] << "," << pipes_out[1] << "]" << std::endl;
   
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "DEBUG: fork() failed: " << strerror(errno) << std::endl;
        close(pipes_out[0]); 
        close(pipes_out[1]);
        close(fd_in);
        return false;
    }
    
    if (pid == 0) {
        // Child process
        std::cout << "DEBUG: Child process started" << std::endl;
        
        close(pipes_out[0]); // Close read end of output
        // close(fd_in);  // Close write end of input
        
        if (fd_in != -1){
            lseek(fd_in, 0, SEEK_SET);
            dup2(fd_in, STDIN_FILENO);   // CGI reads from stdin
            close(fd_in);
        }
        dup2(pipes_out[1], STDOUT_FILENO); // CGI writes to stdout
        dup2(pipes_out[1], STDERR_FILENO);
        
        // close(pipes_in[0]);
        close(pipes_out[1]);
        
        execCGI(*c->req, config);
        std::cerr << "DEBUG: execCGI failed!" << std::endl;
        exit(1);
    }
    
    // Parent process
    std::cout << "DEBUG: Parent: forked child pid=" << pid << std::endl;
    
    close(pipes_out[1]); // Close write end of output
    // close(pipes_in[0]);  // Close read end of input
    
    // Write POST body to CGI stdin if present
    const std::string& body = c->req->getBody();
    if (!body.empty()) {
        std::cout << "DEBUG: Writing " << body.size() << " bytes to CGI stdin (fd=" 
                 << fd_in << ")" << std::endl;
        ssize_t written = write(fd_in, body.c_str(), body.size());
        if (written < 0) {
            std::cerr << "DEBUG: Failed to write to CGI stdin: " << strerror(errno) << std::endl;
        } else if (written != static_cast<ssize_t>(body.size())) {
            std::cerr << "DEBUG: Partial write: " << written << "/" << body.size() << std::endl;
        } else {
            std::cout << "DEBUG: Successfully wrote " << written << " bytes to CGI" << std::endl;
        }
    } else {
        std::cout << "DEBUG: No body to write to CGI stdin" << std::endl;
    }
    
    close(fd_in); // Close stdin - CGI will see EOF
    std::cout << "DEBUG: Closed CGI stdin, CGI should start processing" << std::endl;
    
    // Set output pipe non-blocking
    fcntl(pipes_out[0], F_SETFL, fcntl(pipes_out[0], F_GETFL) | O_NONBLOCK);
    
    Connection* cgi = new Connection(Connection::CGI);
    cgi->fd = pipes_out[0];
    cgi->parent_fd = c->fd;
    cgi->pid = pid;
    
    conns[pipes_out[0]] = cgi;
    addPoll(pipes_out[0], POLLIN);
    
    std::cout << "DEBUG: CGI setup complete - monitoring fd " << pipes_out[0] 
             << " for output from pid " << pid << std::endl;
    
    return true;
}

bool WebServer::readCGI(Connection* cgi) {
    char buf[4096];
    ssize_t n = read(cgi->fd, buf, sizeof(buf));
    
    std::cout << "DEBUG: CGI read returned " << n << " bytes (fd=" << cgi->fd << ")" << std::endl;
    
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "DEBUG: CGI read would block (normal)" << std::endl;
            return true;
        }
        std::cerr << "DEBUG: CGI read error: " << strerror(errno) << std::endl;
        return false;
    }
    
    if (n == 0) {
        std::cout << "DEBUG: CGI finished (EOF), total output: " 
                 << cgi->read_buf.size() << " bytes" << std::endl;
        return false; // EOF - CGI done
    }
    
    cgi->read_buf.append(buf, n);
    std::cout << "DEBUG: CGI total accumulated: " << cgi->read_buf.size() << " bytes" << std::endl;
    // std::cout << cgi->read_buf << std::endl;
    return true;
}

void WebServer::finishCGI(Connection* cgi) {
    // std::cout << "DEBUG: Finishing CGI for client fd " << cgi->parent_fd << std::endl;
    
    std::map<int, Connection*>::iterator client_it = conns.find(cgi->parent_fd);
    if (client_it == conns.end()) {
        std::cout << "DEBUG: Client connection already closed" << std::endl;
        closeConn(cgi->fd);
        return;
    }
    
    Connection* client = client_it->second;
    
    // Wait for CGI process
    int status;
    pid_t result = waitpid(cgi->pid, &status, WNOHANG);
    
    if (result == 0) {
        std::cout << "DEBUG: CGI process " << cgi->pid << " still running, killing..." << std::endl;
        kill(cgi->pid, SIGTERM);
        usleep(100000); // Wait 100ms
        waitpid(cgi->pid, &status, WNOHANG);
    }
    
    if (WIFEXITED(status)) {
        std::cout << "DEBUG: CGI exited normally with status " << WEXITSTATUS(status) << std::endl;
    } else if (WIFSIGNALED(status)) {
        std::cout << "DEBUG: CGI killed by signal " << WTERMSIG(status) << std::endl;
    }
    
    // std::cout << "DEBUG: Building response from " << cgi->read_buf.size() 
    //          << " bytes of CGI output" << std::endl;
    
    // Build response from CGI output
    client->resp = new Response();
    client->resp->fromCGI(cgi->read_buf);
    client->write_buf = client->resp->build();
    client->write_pos = 0;
    client->state = Connection::WRITING;
    
    // std::cout << "DEBUG: Response ready: " << client->write_buf.size() 
    //          << " bytes, switching client to POLLOUT" << std::endl;
    
    modifyPoll(client->fd, POLLOUT);
    closeConn(cgi->fd);
}

bool WebServer::writeResponse(Connection* c) {
    if (c->write_pos >= c->write_buf.size()) {
        std::cout << "DEBUG: Response complete, closing connection" << std::endl;
        closeConn(c->fd);
        return false;
    }
    
    const char* data = c->write_buf.data() + c->write_pos;
    size_t len = c->write_buf.size() - c->write_pos;
    
    ssize_t n = write(c->fd, data, len);
    // std::cout << data << std::endl;
    
    std::cout << "DEBUG: Wrote " << n << " bytes to fd " << c->fd 
             << " (remaining: " << (len - (n > 0 ? n : 0)) << ")" << std::endl;
    
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;
        }
        std::cerr << "DEBUG: Write error: " << strerror(errno) << std::endl;
        return false;
    }
    
    c->write_pos += n;
    
    if (c->write_pos >= c->write_buf.size()) {
        std::cout << "DEBUG: Full response sent" << std::endl;
        closeConn(c->fd);
        return false;
    }
    
    return true;
}

int WebServer::startSocket(const std::string& host, const std::string& port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error: Failed to create socket" << std::endl;
        return -1;
    }
    
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error: setsockopt SO_REUSEADDR failed" << std::endl;
        close(sockfd);
        return -1;
    }
    
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "Error: Failed to set non-blocking" << std::endl;
        close(sockfd);
        return -1;
    }
    
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    
    if (host.empty() || host == "0.0.0.0" || host == "*") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
            std::cerr << "Error: Invalid host address: " << host << std::endl;
            close(sockfd);
            return -1;
        }
    }
    
    int port_num = 0;
    std::istringstream iss(port);
    iss >> port_num;
    if (iss.fail() || port_num < 0 || port_num > 65535) {
        std::cerr << "Error: Invalid port: " << port << std::endl;
        close(sockfd);
        return -1;
    }
    addr.sin_port = htons(port_num);
    
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: bind failed on " << host << ":" << port 
                  << " - " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    
    if (listen(sockfd, 128) < 0) {
        std::cerr << "Error: listen failed - " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

void WebServer::execCGI(const Request& req, ServerConf& config) {
    std::map<std::string, std::string> env;
    
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_SOFTWARE"] = "webserv/1.0";
    env["REQUEST_METHOD"] = req.getStrMethod();
    env["SCRIPT_NAME"] = req.getFullUri();
    env["QUERY_STRING"] = req.getUriQueries();
    env["CONTENT_TYPE"] = req.getHeader("content-type");
    env["REDIRECT_STATUS"] = "200";
    
    std::ostringstream oss;
    oss << req.getBody().size();
    env["CONTENT_LENGTH"] = oss.str();

    std::vector<std::pair<std::string, std::string> >listen = config.getListen();
    std::vector<std::pair<std::string, std::string> >::iterator itt = listen.begin();
    oss.str("");
    oss << itt->second;
    env["SERVER_PORT"] = oss.str();
    
    std::set<std::string> name = config.getServerNames();
    std::set<std::string>::iterator it = name.begin();
    env["SERVER_NAME"] = *it;
    
    env["REMOTE_ADDR"] = req.getHost();
    
    const std::map<std::string, std::string>& headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        if (it->first == "content-type" || it->first == "content-length"){
            continue;
        }

        std::string key = "HTTP_" + it->first;
        for (size_t i = 5; i < key.length(); i++) {
            if (key[i] == '-')
                key[i] = '_';
            else if (key[i] >= 'a' && key[i] <= 'z')
                key[i] = key[i] - 32;
        }
        env[key] = it->second;
    }
    
    std::string scriptPath = req.getFullPath();
    env["SCRIPT_FILENAME"] = scriptPath;

    std::map<std::string, LocationConf> locations = config.getLocations();
    LocationConf conf = locations[req.getLocation()];
    std::map<std::string, std::string> cgis = conf.getCgi();
    std::string interpreter = cgis[req.getCgiType()];
    
    // std::cout << "DEBUG: CGI script: " << scriptPath << std::endl;
    // std::cout << "DEBUG: CGI interpreter: " << interpreter << std::endl;
    // std::cout << "DEBUG: CONTENT_LENGTH: " << env["CONTENT_LENGTH"] << std::endl;
    
    char** envp = new char*[env.size() + 1];
    size_t i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env.begin();
         it != env.end(); ++it, ++i) {
        std::string envStr = it->first + "=" + it->second;
        envp[i] = new char[envStr.length() + 1];
        std::strcpy(envp[i], envStr.c_str());
    }
    envp[i] = NULL;
    
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
    
    // std::cerr << "DEBUG: execve failed - " << strerror(errno) << std::endl;
    
    for (size_t j = 0; j < env.size(); j++) {
        delete[] envp[j];
    }
    delete[] envp;
}