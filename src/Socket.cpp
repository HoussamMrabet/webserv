#include "../includes/Socket.hpp"

Socket::Socket(std::string host, std::string port): _host(host), _port(port) 
{
    std::cout << "Creating Socket!" << std::endl;
    // start socket:
    this->_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (this->_fd < 0){ 
        // check if strerror is allowed!!!
        std::cerr << "Failed to create socket." << std::endl;
        std::cerr << "Errno " << errno << ": " << std::strerror(errno) << std::endl;
        // throw exception!!!
    }
    this->_address.sin_family = AF_INET;
    this->_address.sin_addr.s_addr = INADDR_ANY; 
    this->_address.sin_port = htons(atol(_port.c_str())); 
    // uint16_t htons(uint16_t hostshort);
    
    // bind socket:
    if (bind(this->_fd,(sockaddr *)&this->_address, sizeof(_address)) < 0){
        // check if strerror is allowed!!!
        std::cerr << "Failed to connect socket to port." << std::endl;
        std::cerr << "Errno " << errno << ": " << std::strerror(errno) << std::endl;
        // throw exception !!!
    }
    if (listen(this->_fd, 20) < 0){  
        std::cerr << "Failed to listen on socket." << std::endl;
        std::cerr << "Errno " << errno << ": " << std::strerror(errno) << std::endl;
        // throw exception !!!
    }
}

Socket::~Socket(){
    std::cout << "Closing Socket!" << std::endl;
    close(this->_fd);
}

int Socket::getFd() const{
    return (this->_fd);
}

// struct pollfd Socket::addnew(){
//     struct pollfd newfd;
//     newfd.fd = accept(this->_socket, (sockaddr *)&this->_address, 
//     &this->_addressSize); 
//     if (newfd.fd == -1) {
//         std::cerr << "Error accepting connection\n";
//     }
//     newfd.events = POLLIN;
//     return newfd;
// }

// void Socket::addClient(){
//     std::vector<struct pollfd> fds;
//     struct pollfd sfd;
//     sfd = addnew(); 
//     fds.push_back(sfd);  

//     while (true){
//         // Poll for events (waiting indefinitely for activity)
//         int pollCount = poll(fds.data(), fds.size(), 0);
//         if (pollCount == -1){
//             std::cerr << "Error in poll" << std::endl;
//             break;
//         }

//         // Handle new incoming connection
//         if (fds[0].revents & POLLIN) { // POLLIN: There is data to read.
//             struct pollfd newfd;  
//             newfd.fd = accept(this->_socket, (sockaddr *)&this->_address, 
//             &this->_addressSize); 
//             if (newfd.fd == -1) {
//                 std::cerr << "Error accepting connection\n";
//                 continue;
//             }
//             newfd.events = POLLIN;
//             fds.push_back(newfd);
//             std::cout << "New client connected!" << std::endl;
//         }

//         // Handle incoming data from connected clients
//         for (size_t i = 0; i < fds.size(); ++i){
//             if (fds[i].revents & POLLIN){
//                 this->_newSocket = fds[i].fd;
//                 readMessage();  // Process and print the message immediately when received
//                 std::cout << "Processed request from client " << i << std::endl;
//                 // fds.erase(fds.begin() + i);
//                 // i--;
//                 // unlink();
//             }
//         }
//     }
// }
