#include "Connection.hpp"

bool Connection::readRequest(){
    char buffer[1024];
    ssize_t n = read(_fd, buffer, sizeof(buffer));

    std::cout << "readRequest: n = " << n << ", isDone = " << _request.isDone() << "\n";

    if (n > 0) {
        _buffer.append(buffer, n);
        // Feed the newly read bytes to the request parser
        _request.parseRequest(_buffer);
        _buffer.clear();
        updateTimout();
        std::cout << "After parseRequest: isDone = " << _request.isDone() << "\n";
        
        // Check if request is now complete after parsing this data
        if (_request.isDone()) {
            std::cout << "Request complete after reading data, transitioning to POLLOUT\n";
            getRquestType();
            return false;  // Transition to POLLOUT immediately
        }
    }
    else if (n == 0) {
        // Peer closed connection: try to finalize parsing with any remaining data
        std::cout << "Connection closed by peer (n=0)\n";
        if (!_buffer.empty()) {
            _request.parseRequest(_buffer);
            _buffer.clear();
        } else {
            // Allow parser to finalize when no more data is coming
            _request.parseRequest();
        }
        
        // After parsing, check if request is complete
        if (_request.isDone()) {
            std::cout << "Request complete after peer closed, transitioning to POLLOUT\n";
            getRquestType();
            updateTimout();
            return false;
        }
        
        // Connection closed but request incomplete - close the connection
        std::cout << "Connection closed before request complete - closing connection\n";
        return false;  // Close connection instead of infinite loop
    }
    else { // n < 0
        // EAGAIN/EWOULDBLOCK - no data available right now
        // Call parseRequest() to finalize GET/DELETE requests
        
        if (!_request.isDone())
        {
            std::cout << "EAGAIN: Calling parseRequest() to finalize request\n";
            _request.parseRequest();
        }
        
        if (_request.isDone())
        {
            std::cout << "EAGAIN: Request is DONE, transitioning to POLLOUT\n";
            getRquestType();
            updateTimout();
            return false;
        }
        
        std::cout << "EAGAIN: Waiting for more data (POST body)\n";
        // Still waiting for more data (POST body)
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
