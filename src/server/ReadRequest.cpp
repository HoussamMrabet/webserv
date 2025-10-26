#include "Connection.hpp"

bool Connection::readRequest(){
    char buffer[1024];
    ssize_t n = read(_fd, buffer, sizeof(buffer));

    if (n > 0) {
        _buffer.append(buffer, n);
        // Feed the newly read bytes to the request parser
        _request.parseRequest(_buffer);
        _buffer.clear();
        updateTimout();
    }
    else if (n == 0) {
        // Peer closed connection: try to finalize parsing with any remaining data
        if (!_buffer.empty()) {
            _request.parseRequest(_buffer);
            _buffer.clear();
        } else {
            // Allow parser to finalize when no more data is coming
            _request.parseRequest();
        }
        updateTimout();
    }
    else { // n < 0
            // No data right now — non-blocking: return and wait for next POLLIN
        std::cout << "***************************\n";
        if (_request.getStatusCode() != 200 || _request.isDone())
        {
            getRquestType();
            updateTimout();
            return false;
        }
        return true;
    }

    _done = _request.isDone();
    updateTimout();
    if (_request.isDone())
        getRquestType();
    return true;
}

void Connection::getRquestType(){

    _request.processRequest();
    if (_request.isCGI() && (_request.getStatusCode() == 200) && !_cgi.execDone()){
        try{
            _cgi.executeCGI(_request, _server);
            _cgiFd = _cgi.getFd();
            updateTimout();/////
        }
        catch (const std::exception& e){
            _request.setStatusCode(500);
            _request.CGIError();
            close(_cgiFd);
            std::cerr << "Error: " << e.what() << B"\n";
        }
    }
    requestInfo(_host, _port,
        _request.getStatusCode(), \
        _request.getStrMethod(), \
        _request.getUri(), \
        _request.getHeader("httpVersion"));
}

void Connection::readCGIOutput(){ _response = _cgi.readOutput();}
