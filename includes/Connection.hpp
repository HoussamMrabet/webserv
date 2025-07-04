// #pragma once

// #include <string>
// #include <vector>
// #include <map>
// #include <sys/types.h>
// #include "Request.hpp"
// #include "Response.hpp"
// #include "ServerConf.hpp"

// // Represents a single client connection, handles IO + parsing + response logic
// class Connection {
// public:
//     Connection(int fd, const ServerConfig& cfg);
//     ~Connection();

//     int getFd() const noexcept;
//     bool wantsToRead() const noexcept;
//     bool wantsToWrite() const noexcept;
//     bool isClosed() const noexcept;

//     void handleIO();
//     void handleRead();
//     void handleWrite();

// private:
//     int _fd;
//     const ServerConfig& _config;

//     std::string _inBuffer;
//     std::string _outBuffer;
//     HttpRequest _request;
//     HttpResponse _response;

//     bool _requestComplete;
//     bool _responseReady;
//     bool _closed;

//     void parseRequest();
//     void generateResponse();
//     void resetForNextRequest();
// };
