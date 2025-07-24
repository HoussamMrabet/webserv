#pragma once
// #include "Socket.hpp"    
#include "Listen.hpp"  
#include "ServerConf.hpp"


class ServerBlock{
    private:
        // std::vector<Server> servers;
        // std::vector<Socket> _sockets;
        
        Listen _listen; 
        std::vector<ServerConf> _servers;

    
    public:
        // ServerBlock(std::vector<ServerConf>&);
        ServerBlock();
        ServerBlock(Listen&);
        ServerBlock(const ServerBlock&);
        void addServer(ServerConf&);
        Listen getListen() const;
        std::vector<ServerConf>& getServers();
        // void makeBloks(std::vector<ServerConf>&);
        // static void addSocket(Socket& s);
        // Server(std::string ipAddress, int port);
        // ~Server();
};
std::map<Listen, ServerBlock> makeBloks(std::vector<ServerConf>&);