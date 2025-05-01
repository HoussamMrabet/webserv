#pragma once

class Server{
    public: // fix canonical form
        Server(std::string name, std::string ip, int port);
        ~Server();
    private:
        std::string _name;
        std::string _ip;
        int _port;
};

