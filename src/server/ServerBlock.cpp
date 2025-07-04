#include "ServerBlock.hpp"

ServerBlock::ServerBlock(){}

ServerBlock::ServerBlock(Listen& l):_listen(l){}

ServerBlock::ServerBlock(const ServerBlock& block){
    _listen = block._listen; 
    _servers = block._servers;
}

void ServerBlock::addServer(ServerConf& server){
    _servers.push_back(server);
}

Listen ServerBlock::getListen() const{
    return (_listen);
}

std::vector<ServerConf>& ServerBlock::getServers(){
    return (_servers);
}