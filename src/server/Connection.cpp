#include "Connection.hpp"

Connection::Connection(int fd, ServerConf& server, const std::string& host, const std::string& port): _fd(-1),
                                                    _host(host), _port(port),
                                                    _time(time(NULL)),
                                                    _done(false),
                                                    _responseDone(false),
                                                    _isChunkedResponse(false),
                                                    _currentChunk("") {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    _fd = accept(fd, (struct sockaddr*)&addr, &len);
    if (_fd == -1){
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return ;
        } else{
            throw std::runtime_error("Accept failed");
        }
    }
    _server = server;
    _cgiFd = -1;
    if (!setNonBlocking())
        throw std::runtime_error("fcntl failed");
    updateTimout();
}

Connection::Connection(const Connection& connection){
    _fd = connection._fd;
    _time = connection._time;
    _buffer = connection._buffer;
    _done = connection._done;
    _responseDone = connection._responseDone;
    _isChunkedResponse = connection._isChunkedResponse;

    _cgiFd = connection._cgiFd;
    _host = connection._host;
    _port = connection._port;
    _buffer = connection._buffer;
    _request = connection._request;
    _response = connection._response;
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

ServerConf Connection::getServer(){ return (_server);}

bool Connection::isDone(){ return (_done);}

bool Connection::isResponseDone(){ return (_responseDone);}

time_t Connection::getTime() const { return _time; }

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

Connection::~Connection(){}

void Connection::requestInfo(const std::string& host, const std::string& port, int status, const std::string& method, const std::string& path, const std::string& version) {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "[%a %b %d %H:%M:%S %Y]", t);
    std::cout << M"" << buf << " [" << host << "]:" << port  << " [" << status << "]: "
              << method << " " << path << " " << version << B"" << std::endl;
}
bool Connection::cgiDone(){ return (_cgi.readDone());}