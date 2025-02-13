#include "server.hpp"

Server::Server(std::string ipAddress, int port):   _ipAddress(ipAddress),
                                                    _port(port),
                                                    _socket(),
                                                    _newSocket(),
                                                    _incomingMessage(),
                                                    _socketAddress(),
                                                    _socketAddressSize(sizeof(_socketAddress)),
                                                    _serverMessage("message receeived"){
    std::cout << "Starting server!" << std::endl;
    startServer(); // add exception in case it returns 1
    startListen(); // find its correct place
    acceptConnection(_newSocket); // check and find correct place
    readMessage(); // read message and send response
}

Server::~Server(){
    std::cout << "Closing server!" << std::endl;
    closeServer();
}

int Server:: startServer(){
    this->_socket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket (IPv4, TCP)
    if (this->_socket < 0){ // == -1
        std::cerr << "Failed to create socket." << std::endl;
        std::cerr << "Errno " << errno << ": " << std::strerror(errno) << std::endl;
        // exit(EXIT_FAILURE);
        return (1); // use return, exit or exception??
    }
    // Listen to port 1337 on any address
    this->_socketAddress.sin_family = AF_INET;
    this->_socketAddress.sin_addr.s_addr = INADDR_ANY; // just for test, change it later to:                                                    
    // this->_socketAddress.sin_addr.s_addr = inet_addr(_ipAddress.c_str());                                                        
    this->_socketAddress.sin_port = htons(_port); // htons is necessary to convert a number to network byte order

    if (bind(this->_socket,(sockaddr *)&this->_socketAddress, this->_socketAddressSize) < 0){
        std::cerr << "Failed to connect socket to port." << std::endl;
        std::cerr << "Errno " << errno << ": " << std::strerror(errno) << std::endl;
        // exit(EXIT_FAILURE);
        exit(1);
        return (1);
    }
    return (0);
}

void Server::closeServer(){ // Close the connections
    close(this->_socket); // close fd
    close(this->_newSocket);
    exit(0);
}

void Server::startListen(){
    if (listen(this->_socket, 20) < 0){  // Start listening. Hold at most 20 connections in the queue
        std::cerr << "Failed to listen on socket." << std::endl;
        std::cerr << "Errno " << errno << ": " << std::strerror(errno) << std::endl;
        // exit(EXIT_FAILURE);
    }

    // std::ostringstream ss;
    // ss << "\n*** Listening on ADDRESS: " 
    //     << inet_ntoa(this->_socketAddress.sin_addr) 
    //     << " PORT: " << ntohs(this->_socketAddress.sin_port) 
    //     << " ***\n\n";
    // std::cout << ss.str() << std::endl;
}

void Server::acceptConnection(int &newSocket)
{
    newSocket = accept(this->_socket, (sockaddr *)&this->_socketAddress, 
                        &this->_socketAddressSize); // accept a connection from the queue
    if (newSocket < 0)
    {
        // std::ostringstream ss;
        // ss << 
        // "Server failed to accept incoming connection from ADDRESS: " 
        // << inet_ntoa(this->_socketAddress.sin_addr) << "; PORT: " 
        // << ntohs(this->_socketAddress.sin_port);
        // std::cout << ss.str() << std::endl;
        // exit(1);
        std::cerr << "Failed to accept connection." << std::endl;
        std::cerr << "Errno" << errno << ": " << std::strerror(errno) << std::endl;
        // exit(EXIT_FAILURE);
        exit(1); 
    }
}

void Server::readMessage(){
    // Read from the connection
    char buffer[100];
    this->_incomingMessage = read(this->_newSocket, buffer, 100);
    std::cout << "Client message: " << buffer;

    send(this->_newSocket, this->_serverMessage.c_str(), this->_serverMessage.size(), 0); // Send a message to the connection
}