// #include "Socket.hpp"    
#include "Listen.hpp"  
#include "ServerConf.hpp"

// a serverBlock collects all servers having the same listen
// only one socket is created for each serverBlock
class ServerBlock{

    private:
        Listen _listen; 
        std::vector<ServerConf> _servers;

    
    public:
        // fix code structure!!
        ServerBlock();
        ServerBlock(Listen&);
        ServerBlock(const ServerBlock&);
        void addServer(ServerConf&);
        Listen getListen() const;
        std::vector<ServerConf>& getServers();

};