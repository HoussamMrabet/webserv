#include "../includes/Server.hpp"

Server::Server(std::string name, std::string ip, int port):   _name(name),
                                                    _ip(ip),
                                                    _port(port)
{
    std::cout << "Starting server!" << std::endl;
}

Server::~Server(){
    std::cout << "Closing server!" << std::endl;
}
