#include "Connection.hpp"
ServerConf Connection::_server = ConfigBuilder::generateServers("config/config.conf").back();

#define DEFAULT_RESPONSE "HTTP/1.1 200 OK\r\n" \
                         "Content-Length: 10\r\n" \
                         "Content-Type: text/plain\r\n" \
                         "Connection: keep-alive\r\n" \
                         "\r\n" \
                         "Received!!\r\n" // Connection keep-alive but still ends !!!!!


Connection::Connection(int fd, ServerConf& server): _time(time(NULL)), _request(new Request()),
                                                    _done(false)/*....*/ {
    _fd = accept(fd, NULL, NULL);
    (void)server;
    std::cout << "Connection constructor fd " << _fd << "\n";
    std::cout << _server.getRoot() << std::endl;
    if (_fd == -1) {
        perror("Accept failed");
        exit(1);/// check !!
    }
    setNonBlocking();
    std::cout << "New client connected!\n" << std::endl;
}

Connection::Connection(const Connection& connection){
    _fd = connection._fd;
    _time = connection._time;
    _buffer = connection._buffer;
    _request = connection._request;
    _done = connection._done;
}

int Connection::getFd() const{ return (_fd);}

bool Connection::readRequest(){
    char buffer[1024] = {0};
    ssize_t bytesRead = 0;

    while (!_request->isDone())
    {
        bytesRead = read(_fd, buffer, sizeof(buffer));
        if (bytesRead > 0)
        {
            _buffer.append(buffer, bytesRead);
            _request->parseRequest(_buffer);
            _time = time(NULL);  // update activity timestamp
            _buffer.clear();
            // else continue;
        }
        else
        {
            _request->parseRequest();
            // _buffer.clear();
            std::cout << "Client disconnected!" << std::endl;
            // return (false);
        }
        // else
        // {
        //     std::cout << "Error reading from socket" << std::endl;
        //     // check for errno!!!
        //     perror("Read failed");
        //     return (false);
        // }
    }
    _done = _request->isDone();
    // _request->printRequest();


    // if (_request->isDone()) // remove from here and add to webserv in order to add pipe_fds to pollfds
    //     _response = CGI::executeCGI(*_request);
    return (true);
}

bool Connection::writeResponse(){ // check if cgi or not, if cgi call cgiResponse!!!
    if (getRequestMethod() == "GET"){
        sendGetResponse();
        return (true);
    }
    // if (_response.empty())
    _response = DEFAULT_RESPONSE;
        // _response = "Response sent from server!!!\r\n";
    // std::cout << "status code : " << _request->getStatusCode() << std::endl;
    // _response = Response::getResponse(_request->getStatusCode());
    // ssize_t b = ;
    if (write(_fd, _response.c_str(), _response.length()) == -1){
        perror("Write failed");
        return (false);
    } // send part by part using a buffer, while sum of buffer size sent less that responce size?
    return (true);
}

std::string Connection::getRequestMethod(){
    t_method method = _request->getMethod();
    switch (method)
    {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case DELETE:
            return "DELETE";
        default:
            return "UNDEFINED";
    }
}

void Connection::sendGetResponse(){
    char pwd[100];
    getcwd(pwd, 100);
    std::cout << "---> fd = " << _fd << std::endl;
    std::string path = _server.getRoot();
    std::cout << "------ Path = " << path << std::endl;
    std::vector<std::string> indexes = _server.getIndex();
    std::string index = indexes[0];
    std::cout << "------ index = " << index << std::endl;
    std::string full_path = pwd + path + index;
    std::cout << "------ full_path = " << full_path << std::endl;
    std::ifstream file(full_path.c_str(), std::ios::in | std::ios::binary);
    std::ostringstream get_file;
    get_file << file.rdbuf();
    
    std::ostringstream get_response;
    get_response << "HTTP/1.1 200 OK\r\n";
    get_response << "Content-Type: text.html\r\n";
    get_response << "Content-Lenght:" << get_file.str().size() << "\r\n";
    get_response << "Connection: keep-alive\r\n";
    get_response << "\r\n";
    get_response << get_file.str();
    
    _response = get_response.str();
    std::cout << "------ response = " << _response << std::endl;
    send(_fd, _response.c_str(), _response.size(), 0);
}

void Connection::printRequest(){
    if (_done){
        std::cout << "-------> Received: <----------\n";
        _request->printRequest();
        std::cout << "-------------------------------\n";
    }
    // else
    //     std::cout << "Request not received!\n";
}

ServerConf Connection::getServer(){ return (_server);}

bool Connection::isDone(){ return (_done);}

time_t Connection::getTime() const { return _time; }

void Connection::setNonBlocking() {
    // int flags = fcntl(_fd, F_GETFL, 0);// F_GETFL = get file status flags
    // fcntl(_fd, F_SETFL, flags | O_NONBLOCK); // then change to non bloking
    // fcntl(_fd, F_SETFL, O_NONBLOCK);
    fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK);

}