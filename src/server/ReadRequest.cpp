#include "Connection.hpp"

bool Connection::readRequest(){
    char buffer[1024] = {0};
    ssize_t bytesRead = 0;

    if (_request.isDone()) return (true);
    else {
        bytesRead = read(_fd, buffer, sizeof(buffer));
        if (bytesRead > 0){
            _buffer.append(buffer, bytesRead);
            _request.parseRequest(_buffer);
            _buffer.clear();
            updateTimout();
        }
        else
            _request.parseRequest();
    }
    _done = _request.isDone();
    if (_request.isDone())
        getRquestType();
    return (true);
}

void Connection::getRquestType(){

    _request.processRequest();
    if (_request.isCGI() && (_request.getStatusCode() == 200) && !_cgi.execDone()){
        try{
            _cgi.executeCGI(_request, _server);
            _cgiFd = _cgi.getFd();
        }
        catch (const std::exception& e){
            _request.setStatusCode(500);
            _request.CGIError();
            close(_cgiFd);
            std::cerr << R"Error: " << e.what() << B"\n";
        }
    }
    requestInfo(_host, _port,
        _request.getStatusCode(), \
        _request.getStrMethod(), \
        _request.getUri(), \
        _request.getHeader("httpVersion"));
}

void Connection::readCGIOutput(){ _response = _cgi.readOutput();}
