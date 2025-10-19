#include "../includes/Socket.hpp"


int Socket::StartSocket(const std::string& host, const std::string& port){
    Socket socket(host, port);

    socket.socket_start();
    socket.socket_fcntl();
    socket.socket_bind();
    socket.socket_listen();
    return (socket._fd);
}

Socket::~Socket(){
    // delete _ip;
}

Socket::Socket(const std::string& host, const std::string& port):_fd(-1), _host(host), _port(port){}

void Socket::socket_start(){  // useless!!
    
    memset(&_info, 0, sizeof(_info));
    _info.ai_family = AF_INET;  // IPv4 add
    _info.ai_socktype = SOCK_STREAM; // tcp socket
    /* The available socket types:
    //  - SOCK_STREAM: Provides a reliable, byte-stream-based communication (TCP)
    //  - SOCK_DGRAM: Provides a connectionless, unreliable communication (UDP)
    //  - SOCK_RAW: Provides raw network access (used in special cases like IP-level communication)
    //  - SOCK_SEQPACKET: Provides a connection-oriented, reliable communication with record boundaries
    */
    if (int err = getaddrinfo(_host.c_str(), _port.c_str(), &_info, &_ip)) {
        std::cerr << "getaddrinfo error: " << gai_strerror(err) << std::endl;
        exit(1);  // Exit if there was an error or exception??
    } // dont forget to free the memory allocated by getaddrinfo() !!!
    // freeaddrinfo(ip);
    
    if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // -1
        perror("Socket failed");
        exit(1); // manage failiar
    }
    
    int val = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) < 0){ // reuse port
        std::cerr << "Error setting socket option" << std::endl;
        close(_fd);  // Close the socket on error
        exit(1);  // cleanup before exit!!!
    }
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){ // set the socket options
        // SO_REUSEADDR allows address reuse when binding a socket to a port that was recently closed
        // SOL_SOCKET level at which the option resides
        // SOL_SOCKET = socket level
        std::cerr << "Error setting socket option" << std::endl;
        close(_fd);  // Close the socket on error
        exit(1);  // cleanup before exit!!!
    }
}

void Socket::socket_fcntl(){
    // int flags = fcntl(sockfd, F_GETFL, 0);// F_GETFL = get file status flags
    // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); // then change to non bloking
    fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK);
}


void Socket::socket_bind(){
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    std::stringstream ss(_port);
    int port; ss >> port;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (inet_pton(AF_INET, _host.c_str(), &address.sin_addr) <= 0) {
        perror("Invalid address");
        close(_fd);
        exit(1);
    }
    if (bind(_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("Bind failed");
        close(_fd);
        exit(1);
    }
    freeaddrinfo(_ip);
}

void Socket::socket_listen(){
    if (listen(_fd, MAXCONNECTIONS) == -1) {
        perror("Listen failed");
        close(_fd);
        exit(1);
    }
}
