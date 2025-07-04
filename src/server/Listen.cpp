#include "Listen.hpp"

Listen::Listen(){}

Listen::Listen(std::string& host, std::string& port): _host(host), _port(port){}

std::string Listen::getHost() const{ return (_host);}

std::string Listen::getPort() const{ return (_port);}

int Listen::getPort(int) const{
    std::stringstream ss(_port);
    int port; ss >> port;
    return (port);
}

bool Listen::operator==(const Listen& l) const{
    return (_host == l._host && _port == l._port);
}
bool Listen::operator<(const Listen& l) const{
    if (_host < l._host) return (true);
    if (_host > l._host) return (false);
    return (_port < l._port);
}