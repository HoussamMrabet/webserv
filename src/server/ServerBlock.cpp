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

std::map<Listen, ServerBlock> makeBloks(std::vector<ServerConf>& servers){
std::map<Listen, ServerBlock> _serverBlocks;
    for (size_t i = 0; i < servers.size(); i++) {
        std::vector<std::pair<std::string, std::string> > _listen = servers[i].getListen();
        bool added = false;
        for (size_t j = 0; j < _listen.size(); j++) {
            Listen tmp(_listen[j].first, _listen[j].second);
            bool exist = false;
            
            if (_serverBlocks.size()) {
                // check if the listen block already exists in _serverBlocks:
                std::map<Listen, ServerBlock >::iterator it = _serverBlocks.find(tmp);
                if (it != _serverBlocks.end() && !added) {
                    // If it exists, add the server to the existing block
                    it->second.addServer(servers[i]);
                    exist = true;
                }
            }

            // If the listen block doesn't exist, create a new one
            if (!exist) {
                ServerBlock block(tmp);
                block.addServer(servers[i]);
                _serverBlocks[tmp] = block;
                added = true;
            }
        }
    }
    return (_serverBlocks);
}