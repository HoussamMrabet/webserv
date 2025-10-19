#include "Connection.hpp"

// ServerConf Connection::_server;
// ServerConf Connection::_server = ConfigBuilder::generateServers("config/config.conf").back();


Connection::~Connection(){ 
    CHOROUK && std::cout << "***********  connection destructor called!!! ***********\n";
    // delete _request;
}

void Connection::requestInfo(const std::string& host, const std::string& port, int status, const std::string& method, const std::string& path, const std::string& version) {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "[%a %b %d %H:%M:%S %Y]", t);
    // std::cout << M"" << buf << " [" << _server.getListen().begin()->first
            //   << "]:" << _server.getListen().begin()->second  << " [" << status << "]: "
    std::cout << M"" << buf << " [" << host << "]:" << port  << " [" << status << "]: "
              << method << " " << path << " " << version << B"" << std::endl;
}

Connection::Connection(int fd, ServerConf& server, const std::string& host, const std::string& port): _fd(-1),
                                                    _host(host), _port(port),
                                                    _time(time(NULL)),
                                                    _done(false),
                                                    _responseDone(false),
                                                    _isChunkedResponse(false)/*....*/ {
    // _fd = accept(fd, NULL, NULL);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    _fd = accept(fd, (struct sockaddr*)&addr, &len);
    if (_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No incoming connections right now - just return false or ignore
            return ;
        } else {
            perror("Accept failed");
            return ; // or handle fatal error
        }
    }
    _server = server;
    // _request = new Request();
    _cgiFd = -1;
    // std::cout << "Connection constructor fd " << _fd << "\n";
    // std::cout << _server.getRoot() << std::endl;
    if (_fd == -1) {
        perror("Accept failed");
        // exit(1);/// check !!
    }
    setNonBlocking();
    // std::cout << "New connection (fd " << _fd << ")\n" << std::endl;
    // std::cout << "[Sun Sep 14 15:23:11 2025] [::1]:52214 [302]: " << _scriptFileName << "\n" << std::endl;
    // std::cout << "[Sun Sep 14 15:23:11 2025] [::1]:52214 [302]: ";
}

Connection::Connection(const Connection& connection){
    _fd = connection._fd;
    _time = connection._time;
    _buffer = connection._buffer;
    // _request = connection._request;
    _done = connection._done;
    _responseDone = connection._responseDone;
    _isChunkedResponse = connection._isChunkedResponse;

    _cgiFd = connection._cgiFd;
    _host = connection._host;
    _port = connection._port;
    _buffer = connection._buffer;
    _request = connection._request;
    // _response_obj = connection._response_obj;  // For chunked responses
    _response = connection._response;   // For simple response = connection.s
    _server = connection._server;

}

int Connection::getFd() const{ return (_fd);}

bool Connection::readRequest(){
    char buffer[1024] = {0};
    ssize_t bytesRead = 0;

    if (_request.isDone()) return (true);
    // if  (!_request.isDone()/* && this->_responseDone == false*/)
    else {
        CHOROUK && std::cout << "------- read fd = " << _fd << std::endl;
        bytesRead = read(_fd, buffer, sizeof(buffer));
        CHOROUK && std::cout << "------- bytes = " << bytesRead << std::endl;
        if (bytesRead > 0)
        {
            _buffer.append(buffer, bytesRead);
            _request.parseRequest(_buffer);
            _buffer.clear();
            updateTimout();  // update activity timestamp
            // else continue;
        }
        else
        {
            // _request.processRequest();
            _request.parseRequest();
            // _buffer.clear();
            // std::cout << "Client disconnected!" << std::endl;
            // _done = true;
            // _responseDone = true;
            // return (false);
        }
        // if (bytesRead < 1024){
        //     _done = true;
        //     return (true);
        // } 
        // else
        // {
        //     std::cout << "Error reading from socket" << std::endl;
        //     // check for errno!!!
        //     perror("Read failed");
        //     return (false);
        // }
    }
    _done = _request.isDone();
    if (_request.isDone()){
        CHOROUK && std::cout << "--> bytesRead = " << bytesRead << std::endl;
        requestInfo(_host, _port,
            _request.getStatusCode(), \
            _request.getStrMethod(), \
            _request.getUri(), \
            _request.getHeader("httpVersion"));

        _request.processRequest();
        // if (_request.isCGI())
            // _cgiFd = 
        // std::cout << "+++++++++++++++++++++++++++++\n";
        // std::cout << _request.isCGI() << std::endl;
        // std::cout << "+++++++++++++++++++++++++++++\n";
        // _request.parseRequest();
    }
    // CHOROUK && _request.printRequest();


    // if (_request.isDone()) // remove from here and add to webserv in order to add pipe_fds to pollfds
    //     _response = CGI::executeCGI(_request);
    return (true);
}

bool Connection::writeResponse(){ // check if cgi or not, if cgi call cgiResponse!!!
    // First, check if we're in the middle of sending a chunked response

    CHOROUK && std::cout << "------- write fd = " << _fd << std::endl;
    // std::cout << M"" << _request.getUri() << B"\n";
    // requestInfo(_request.getStatusCode(), \
    //         _request.getStrMethod(), \
    //         _request.getUri(), \
    //         _request.getHeader("httpVersion"));

    if (_isChunkedResponse) {
        // Continue sending chunks from existing response
        if (!_response_obj.isFinished()) {
            std::string chunk = _response_obj.getResponseChunk();
            if (!chunk.empty()) {
                ssize_t bytes_sent = write(_fd, chunk.c_str(), chunk.length());
                if (bytes_sent == -1) {
                    perror("Chunked write failed");
                    return (false);
                } else if (bytes_sent == 0) {
                    MOHAMED && std::cout << "Connection closed by peer during chunked transfer" << std::endl;
                    return (false);
                } else if (bytes_sent < (ssize_t)chunk.length()) {
                    // Partial write - handle this case
                    MOHAMED && std::cout << "Partial write: " << bytes_sent << "/" << chunk.length() << " bytes" << std::endl;
                    return (false);
                }
                MOHAMED && std::cout << "Sent chunk of size: " << bytes_sent << std::endl;
                return (true); // Successfully sent a chunk, return
            }
        } else {
            // All chunks sent, mark response as done
            MOHAMED && std::cout << "Chunked response complete" << std::endl;
            _responseDone = true;
            _isChunkedResponse = false;
            return (true);
        }
    }
    
    // Regular response processing - only if not in chunked mode
    // Check for redirects first
    std::string redirect_url = checkForRedirect(_request, _server);

    // CHOROUK && std::cout << C"--------- REDIRECTION FOUND!!!! -----------" << B"\n";
    if (!redirect_url.empty()) {
        _response = sendRedirectResponse(_request, redirect_url, _server);
        MOHAMED && std::cout << "Redirect Response:\n" << _response << std::endl;
        updateTimout();
    }
    else if (_request.isCGI() && _request.getStatusCode() == 200)
    {
        CHOROUK && std:: cout << M"IT IS CGI!!!!!\n";
        CGI cgi(_request, _server);
        _response = cgi.executeCGI();
        _response = setCGIHeaders();
        // _cgiFd = CGI::getFd();
        updateTimout();
    }
    else if ( _request.getStatusCode() != 200){
        MOHAMED && std::cout << "Error status code: " << _request.getStatusCode()  << std::endl;
        MOHAMED && std::cout << "Error message: " << _request.getMessage()  << std::endl;
        sendErrorPage(_request, _request.getStatusCode(), _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "POST"){
        // sentPostResponse(_request, _server);
        sendPostResponse(_request, _request.getStatusCode(), _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "DELETE"){
        sendDeleteResponse(_request, _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "GET"){ // can use pointer to member function 
        sendGetResponse(_request, _server);
        if (!_isChunkedResponse) {
            MOHAMED && std::cout << _response << std::endl;
        }
        updateTimout();
    }
    
    // Handle regular (non-chunked) response sending
    // Only executed if we're not in chunked mode or if chunked response just started
    // if (_response.empty()){
    //     _response = DEFAULT_RESPONSE;
    //     updateTimout();
    // }
    if (!_isChunkedResponse) {
        CHOROUK && std::cout << "writing on fd = " << _fd << std::endl;
        int b = write(_fd, _response.c_str(), _response.length());
        if (b == -1){
            // perror("Write failed"); // throw
            return (false);
        }
        CHOROUK && std::cout << "----------- STILL HERE ---------------\n";
        updateTimout();
        // Response is complete
        _responseDone = true;
    }
    
    return (true);
}

std::string Connection::setCGIHeaders(){
    std::ostringstream response;
    
    size_t header_end = _response.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        header_end = _response.find("\n\n");
        if (header_end != std::string::npos) {
            header_end += 2;
        }
    } else {
        header_end += 4;
    }
    
    if (header_end != std::string::npos) {
        std::string headers_part = _response.substr(0, header_end);
        std::string body_part = _response.substr(header_end);
        
        if (headers_part.find("HTTP/") == 0) {
            return (_response);
        } else {
            response << "HTTP/1.1 200 OK\r\n";
            response << headers_part;
            if (headers_part[headers_part.size() - 1] != '\n') {
                response << "\r\n";
            }
            response << body_part;
        }
    } else {
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << _response.size() << "\r\n";
        response << "\r\n";
        response << _response;
    }
    
    return (response.str());
}

// std::string Connection::getRequestMethod(){
//     t_method method = _request.getMethod();
//     switch (method)
//     {
//         case GET:
//             return "GET";
//         case POST:
//             return "POST";
//         case DELETE:
//             return "DELETE";
//         default:
//             return "UNDEFINED";
//     }
// }


int Connection::getCgiFd() const{ return (_cgiFd);}




void Connection::printRequest(){
    if (_done){
        std::cout << "-------> Request received: <----------\n";
        _request.printRequest();
        std::cout << "-------------------------------\n";
    }
    // else
    //     std::cout << "Request not received!\n";
}

ServerConf Connection::getServer(){ return (_server);}

bool Connection::isDone(){ return (_done);}

bool Connection::isResponseDone(){ return (_responseDone);}

time_t Connection::getTime() const { return _time; }

void Connection::setNonBlocking() {
    // int flags = fcntl(_fd, F_GETFL, 0);// F_GETFL = get file status flags
    // fcntl(_fd, F_SETFL, flags | O_NONBLOCK); // then change to non bloking
    // fcntl(_fd, F_SETFL, O_NONBLOCK);
    fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK);

}

void Connection::updateTimout(){
    _time = time(NULL);
}

std::string Connection::getConnectionHeader(Request &request) {
    std::string connection_header = request.getHeader("Connection");
    
    // Convert to lowercase for case-insensitive comparison
    std::transform(connection_header.begin(), connection_header.end(), connection_header.begin(), ::tolower);
    
    // If the request explicitly asks for close, return close
    if (connection_header == "close") {
        return "close";
    }
    
    // Default to keep-alive for HTTP/1.1
    return "keep-alive";
}

std::string Connection::checkForRedirect(Request &request, ServerConf &server) {
    std::string requested_path = request.getUri();
    std::map<std::string, LocationConf> locations = server.getLocations();
    
    // Find the best matching location (longest prefix match)
    std::string best_match = "";
    const LocationConf* best_location = NULL;
    if ((requested_path == "/profile/profile.html" ) && Request::loggedIn == false)
        return "/profile/login.html";
    if ((requested_path == "/profile" || requested_path == "/profile/"||requested_path == "/profile/login.html") && Request::loggedIn == true)
        return "/profile/profile.html";
    if (requested_path == "/logout")
        return "/profile";
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); 
         it != locations.end(); ++it) {
        if (requested_path.find(it->first) == 0 && it->first.length() > best_match.length()) {
            best_match = it->first;
            best_location = &(it->second);
        }
    }
    
    // If we found a matching location and it has a redirect URL, return it
    if (best_location != NULL) {
        std::string redirect_url = best_location->getRedirectUrl();
        if (!redirect_url.empty()) {
            MOHAMED && std::cout << "Redirect found for path: " << requested_path << " -> " << redirect_url << std::endl;
            return redirect_url;
        }
    }
    
    return ""; // No redirect found
}

std::string Connection::sendRedirectResponse(Request &request, const std::string &redirect_url, ServerConf &server) {
    Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    (void)server; // Suppress unused parameter warning
    
    // Set 301 status code (permanent redirect) 
    response_obj.setStatus(301);
    response_obj.setHeader("Location", redirect_url);
    response_obj.setHeader("Connection", connection_header);
    response_obj.setHeader("Content-Type", "text/html");
    
    std::string redirect_body = "<html><body><h1>301 Moved Permanently</h1>"
                               "<p>The document has moved <a href=\"" + redirect_url + "\">here</a>.</p>"
                               "</body></html>";
    
    response_obj.setBody(redirect_body);
    std::string response = response_obj.buildResponse();
    _isChunkedResponse = false; // Redirects are always small responses
    
    MOHAMED && std::cout << "Sending redirect response to: " << redirect_url << std::endl;
    return response;
}

bool Connection::isCGI() const{ return (_request.isCGI());}

std::string Connection::to_str(int n){
    std::stringstream ss; ss << n;
    return (ss.str());
}