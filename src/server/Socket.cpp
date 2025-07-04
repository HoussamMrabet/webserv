#include "../includes/Socket.hpp"
/* useless headers :*/
#include <arpa/inet.h>  // For inet_ntop
#include <netdb.h>   // For getaddrinfo
/* useless */

Socket::Socket(Listen& listen): _listen(listen){
    socket_init(); // to remove 
    if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // -1
        perror("Socket failed"); // manage failiar!
        exit(1);
    }
    
    int val = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){ // set the socket options
        // SO_REUSEADDR allows address reuse when binding a socket to a port that was recently closed
        // SOL_SOCKET level at which the option resides
        // SOL_SOCKET = socket level
        std::cerr << "Error setting socket option" << std::endl;
        close(_fd);  // Close the socket on error
        exit(1);  // cleanup before exit!!!
    }
}

Socket::~Socket(){
    // std::cout << "Closing Socket!" << std::endl;
    // close(this->_fd); // dont close it yet!!!
}

int Socket::getFd() const{
    return (this->_fd);
}

Listen Socket::getListen() const{ return (_listen);}

void Socket::socket_init(){  // useless!!
    struct addrinfo info, *ip;
    memset(&info, 0, sizeof(info));
    info.ai_family = AF_INET;  // IPv4 add
    info.ai_socktype = SOCK_STREAM; // tcp socket
    /* The available socket types:
    //  - SOCK_STREAM: Provides a reliable, byte-stream-based communication (TCP)
    //  - SOCK_DGRAM: Provides a connectionless, unreliable communication (UDP)
    //  - SOCK_RAW: Provides raw network access (used in special cases like IP-level communication)
    //  - SOCK_SEQPACKET: Provides a connection-oriented, reliable communication with record boundaries
    */
    if (int err = getaddrinfo(_listen.getHost().c_str(), _listen.getPort().c_str(), &info, &ip)) {
        std::cerr << "getaddrinfo error: " << gai_strerror(err) << std::endl;
        exit(1);
    }
}

void Socket::socket_fcntl(){
    // int flags = fcntl(sockfd, F_GETFL, 0);// F_GETFL = get file status flags
    // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); // then change to non bloking
    fcntl(_fd, F_SETFL, O_NONBLOCK);
}

void Socket::socket_start(){ // add to bind 
    // struct sockaddr_in _address;
    memset(&_address, 0, sizeof(_address));
    // std::stringstream ss(_port);
    // int d; ss >> d;
    _address.sin_family = AF_INET;
    _address.sin_port = htons(_listen.getPort(0));
    if (inet_pton(AF_INET, _listen.getHost().c_str(), &_address.sin_addr) <= 0) {
        perror("Invalid address");
        close(_fd);
        exit(1);
    }
}

void Socket::socket_bind(){
    if (bind(_fd, (struct sockaddr*)&_address, sizeof(_address)) == -1) {
        perror("Bind failed");
        close(_fd);
        exit(1);
    }
}

void Socket::socket_listen(){
    if (listen(_fd, INCOMING_CONNECTIONS) == -1) {
        perror("Listen failed");
        close(_fd);
        exit(1);
    }
}
