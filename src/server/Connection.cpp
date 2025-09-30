#include "Connection.hpp"

ServerConf Connection::_server;
// ServerConf Connection::_server = ConfigBuilder::generateServers("config/config.conf").back();


#define DEFAULT_RESPONSE "HTTP/1.1 200 OK\r\n" \
                         "Content-Length: 10\r\n" \
                         "Content-Type: text/plain\r\n" \
                         "Connection: keep-alive\r\n" \
                         "\r\n" \
                         "Received!!\r\n" // Connection keep-alive but still ends !!!!!



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
                                                    _done(false)
                                                    // _responseDone(false),
                                                    // _isChunkedResponse(false)/*....*/ 
                                                    {
    _fd = accept(fd, NULL, NULL);
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
    // _responseDone = connection._responseDone;
    // _isChunkedResponse = connection._isChunkedResponse;
}

int Connection::getFd() const{ return (_fd);}

bool Connection::readRequest(){
    char buffer[1024] = {0};
    ssize_t bytesRead = 0;

    CHOROUK && std::cout << M"" << _request.getUri() << B"\n";

    if  (!_request.isDone())
    {
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
    // CHOROUK && _request.printRequest();


    // if (_request.isDone()) // remove from here and add to webserv in order to add pipe_fds to pollfds
    //     _response = CGI::executeCGI(_request);
    return (true);
}

bool Connection::writeResponse(){ // check if cgi or not, if cgi call cgiResponse!!!
    // First, check if we're in the middle of sending a chunked response

    CHOROUK && std::cout << "------- write fd = " << _fd << std::endl;
    // std::cout << M"" << _request.getUri() << B"\n";
    requestInfo(_host, _port,
                _request.getStatusCode(), \
                _request.getStrMethod(), \
                _request.getUri(), \
                _request.getHeader("httpVersion"));
    // if (_isChunkedResponse) {
    if (_response.isChunked()) {
        // Continue sending chunks from existing response
        if (!_response.isFinished()) {
            std::string chunk = _response.getResponseChunk();
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
            _response.setAsDone();
            // _responseDone = true;
            _response.setAsChunked(false);
            // _isChunkedResponse = false;
            return (true);
        }
    }
    
    // Regular response processing - only if not in chunked mode
    // Check for redirects first
    std::string redirect_url = checkForRedirect(_request, _server);
    // CHOROUK && std::cout << C"--------- REDIRECTION FOUND!!!! -----------" << B"\n";
    if (!redirect_url.empty()) {
        sendRedirectResponse(_request, redirect_url, _server);
        // MOHAMED && std::cout << "Redirect Response:\n" << _response << std::endl;
        updateTimout();
    }
    // else if (_request.isCGI())
    // {
    //     CHOROUK && std:: cout << M"IT IS CGI!!!!!\n";
    //     _response = CGI::executeCGI(_request, _server);
    //     // _cgiFd = CGI::getFd();
    //     updateTimout();
    // }
    else if ( _request.getStatusCode() != 200){
        MOHAMED && std::cout << "Error status code: " << _request.getStatusCode()  << std::endl;
        MOHAMED && std::cout << "Error message: " << _request.getMessage()  << std::endl;
        sendErrorPage(_request, _request.getStatusCode(), _server);
        // MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "POST"){
        // sentPostResponse(_request, _server);
        sendPostResponse(_request, _request.getStatusCode(), _server);
        // MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "DELETE"){
        // sendDeleteResponse(_request, _server);
        sendErrorPage(_request, 501, _server); // not implemented
        // MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "GET"){ // can use pointer to member function 
        sendGetResponse(_request, _server);
        // if (!_isChunkedResponse) {
        if (!_response.isChunked()) {
            // MOHAMED && std::cout << _response << std::endl;
        }
        updateTimout();
    }
    else{ // to check!!!!
        sendErrorPage(_request, 501, _server); // not implemented
        updateTimout();
    }
    // // Handle regular (non-chunked) response sending
    // // Only executed if we're not in chunked mode or if chunked response just started
    // if (_response.empty()){
    //     _response = DEFAULT_RESPONSE;
    //     updateTimout();
    // }
    // if (!_isChunkedResponse) {
    if (!_response.isChunked()) {
        CHOROUK && std::cout << "writing on fd = " << _fd << std::endl;
        int b = write(_fd, _response.getResponse().c_str(), _response.getResponse().length());
        // int b = write(_fd, _response.c_str(), _response.length());
        if (b == -1){
            // perror("Write failed"); // throw
            return (false);
        }
        CHOROUK && std::cout << "----------- STILL HERE ---------------\n";
        updateTimout();
        // Response is complete
        _response.setAsDone();
        // _responseDone = true;
    }
    
    return (true);
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

bool Connection::isCGIRequest(const std::string full_path){
    if (_request.isCGI())
        return (true);
    if (full_path.length() >= 4 && full_path.substr(full_path.length() - 4) == ".php"){
        _request.setCgiType(".php");
        return (true);
    }
    else if (full_path.length() >= 3 && full_path.substr(full_path.length() - 3) == ".py"){
        _request.setCgiType(".py");
        return (true);
    }
    return (false);
    // if ((lastPart.empty() || (lastPart == "cgi-bin")) && \
    //         (this->uri == "/cgi-bin/" || this->uri == "/cgi-bin") /* && auto_index == true*/){

    //             this->cgiType = ".py"; // default cgi file
    // //         // check if cgi index file exists in location 
    // //         // if first doesn't exist check for second or global index file
    // //         if (!indexs.empty()){
    // //             this->uriFileName = indexs[0];
    // //         }
    // //         else if (!server.getIndex().empty()){
    // //             this->uriFileName = server.getIndex()[0]; // global index file
    // //         }
    // //         else {
    // //             this->uriFileName = ""; // default cgi file
    // //         }
    //         // if (this->uri[this->uri.length() - 1] != '/')
    //         //     this->uri += "/";
    //         // this->uri +=  this->uriFileName;
    //         // std::cout << "++++ uri after adding index: " << this->uri << std::endl;
    // }
}



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

bool Connection::isResponseDone(){ return (_response.isDone());}

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

