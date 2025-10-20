#include "Connection.hpp"

// ServerConf Connection::_server;
// ServerConf Connection::_server = ConfigBuilder::generateServers("config/config.conf").back();

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
    if (_fd == -1){
        // if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //     // No incoming connections right now - just return false or ignore
        //     return ;
        // } else{
            throw std::runtime_error("Accept failed");
        //     // return ; // or handle fatal error
        // }
    }
    _server = server;
    _cgiFd = -1;
    if (!setNonBlocking())
        throw std::runtime_error("fcntl failed");
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

// void Connection::setNonBlocking() {
//     // int flags = fcntl(_fd, F_GETFL, 0);// F_GETFL = get file status flags
//     // fcntl(_fd, F_SETFL, flags | O_NONBLOCK); // then change to non bloking
//     // fcntl(_fd, F_SETFL, O_NONBLOCK);
//     fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK);

// }

bool Connection::setNonBlocking(){
    return ((fcntl(_fd, F_SETFL, O_NONBLOCK) != -1));
}

void Connection::updateTimout(){
    _time = time(NULL);
}

bool Connection::isCGI() const{ return (_request.isCGI() && (_request.getStatusCode() == 200));}

std::string Connection::to_str(int n){
    std::stringstream ss; ss << n;
    return (ss.str());
}

Connection::~Connection(){ 
    // CHOROUK && std::cout << "***********  connection destructor called!!! ***********\n";
    // delete _request;
}

void Connection::requestInfo(const std::string& host, const std::string& port, int status, const std::string& method, const std::string& path, const std::string& version) {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "[%a %b %d %H:%M:%S %Y]", t);
    std::cout << M"" << buf << " [" << host << "]:" << port  << " [" << status << "]: "
              << method << " " << path << " " << version << B"" << std::endl;
}
bool Connection::cgiDone(){ return (_cgi.readDone());}