#include "Connection.hpp"

bool Connection::readRequest(){
    char buffer[1024] = {0};
    int n = 0;
    // int totale = 0;

    if (_request.isDone()) return (true);
    else {
        n = read(_fd, buffer, sizeof(buffer));
        if (n > 0){
            _buffer.append(buffer, n);
            _request.parseRequest(_buffer);
            _buffer.clear();
            updateTimout();
        }
        else if (n == 0){

            std::cout <<"-------------\n";
            _request.parseRequest();
            updateTimout();///////

            // exit(0);
        }
        else {
            updateTimout();///////

            // return(true);
        }
        // std::cout << "------> lenght = " << _request.getHeader("content-length") << std::endl;
        // totale += n;
        // if (totale == n) {
        //     exit(0);
        // }
    }
    _done = _request.isDone();
    updateTimout();///////
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
            updateTimout();/////
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
