#include "Connection.hpp"

bool Connection::readRequest(){
    char buffer[1024];
    int n = 0;

    // If request is already done, don't read more
    if (_request.isDone()) {
        CHOROUK && std::cout << "Request already done, fd: " << _fd << std::endl;
        return true;
    }
    
    n = read(_fd, buffer, sizeof(buffer) - 1);
    
    if (n > 0) {
        buffer[n] = '\0'; // Null terminate for safety
        _buffer.append(buffer, n);
        _request.parseRequest(_buffer);
        _buffer.clear();
        updateTimout();
        
        CHOROUK && std::cout << "Read " << n << " bytes from fd: " << _fd << std::endl;
    }
    else if (n == 0) {
        // Client closed connection
        MOHAMED && std::cout << "Client closed connection, fd: " << _fd << std::endl;
        _request.parseRequest(); // Finalize parsing
        return false; // Signal connection should be closed
    }
    else {
        // n < 0, error occurred
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now, try again later
            CHOROUK && std::cout << "Would block on fd: " << _fd << std::endl;
            return true;
        }
        
        // Real error
        perror("Read failed");
        return false;
    }
    
    _done = _request.isDone();
    
    // If request is complete, determine type and start processing
    if (_done) {
        CHOROUK && std::cout << "Request complete on fd: " << _fd << std::endl;
        getRquestType();
    }
    
    return true;
}

void Connection::getRquestType(){
    _request.processRequest();
    
    // If this is a CGI request with success status, execute it
    if (_request.isCGI() && (_request.getStatusCode() == 200) && !_cgi.execDone()){
        try {
            _cgi.executeCGI(_request, _server);
            _cgiFd = _cgi.getFd();
            CHOROUK && std::cout << "CGI execution started, fd: " << _cgiFd << std::endl;
        }
        catch (const std::exception& e) {
            _request.setStatusCode(500);
            _request.CGIError();
            if (_cgiFd != -1) {
                close(_cgiFd);
                _cgiFd = -1;
            }
            std::cerr << R"Error: " << e.what() << B"\n";
        }
    }
    
    // Log the request
    requestInfo(_host, _port,
        _request.getStatusCode(),
        _request.getStrMethod(),
        _request.getUri(),
        _request.getHeader("httpVersion"));
}

void Connection::readCGIOutput() { 
    try {
        _response = _cgi.readOutput();
        CHOROUK && std::cout << "Read CGI output, total size: " << _response.length() << std::endl;
    }
    catch (const std::exception& e) {
        _request.setStatusCode(500);
        std::cerr << R"Error reading CGI: " << e.what() << B"\n";
        _cgi.~CGI(); // Force cleanup
    }
}