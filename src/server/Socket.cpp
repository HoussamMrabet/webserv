#include "Socket.hpp"


int Socket::StartSocket(const std::string& host, const std::string& port){
    Socket socket(host, port);

    socket.socket_start();
    socket.socket_fcntl();
    socket.socket_bind();
    socket.socket_listen();
    return (socket._fd);
}

Socket::~Socket(){}

Socket::Socket(const std::string& host, const std::string& port):_fd(-1), _host(host), _port(port){}

void Socket::socket_start(){
    
    memset(&_info, 0, sizeof(_info));
    _info.ai_family = AF_INET;
    _info.ai_socktype = SOCK_STREAM;
    if (int err = getaddrinfo(_host.c_str(), _port.c_str(), &_info, &_ip)) {
        std::cerr << "Error: " << gai_strerror(err) << std::endl;
        throw std::runtime_error("getaddrinfo failed");
    }
    
    if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        freeaddrinfo(_ip);
        throw std::runtime_error("Socket failed");
    }
    
    int val = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) < 0){
        close(_fd);
        freeaddrinfo(_ip);
        throw std::runtime_error("Set socket options failed");
    }
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){
        close(_fd);
        freeaddrinfo(_ip);
        throw std::runtime_error("Set socket options failed");
    }
}

void Socket::socket_fcntl(){
    if (fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK) == -1){
        close(_fd);
        freeaddrinfo(_ip);
        throw std::runtime_error("fcntl failed");
    }
}


void Socket::socket_bind(){
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    std::stringstream ss(_port);
    int port; ss >> port;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (inet_pton(AF_INET, _host.c_str(), &address.sin_addr) <= 0){
        close(_fd);
        freeaddrinfo(_ip);
        throw std::runtime_error("Invalid address");
    }
    if (bind(_fd, (struct sockaddr*)&address, sizeof(address)) == -1){
        close(_fd);
        freeaddrinfo(_ip);
        throw std::runtime_error("Bind failed");
    }
    freeaddrinfo(_ip);
}

void Socket::socket_listen(){
    if (listen(_fd, MAXCONNECTIONS) == -1) {
        close(_fd);
        throw std::runtime_error("Listen failed");
    }
}
