#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include <iostream>
#include <sys/socket.h> 
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <dirent.h>
#include <vector>
#include <sstream>
#include <errno.h>
#include <arpa/inet.h>
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConf.hpp"
#include "CGI.hpp"

#define LARGE_FILE_THRESHOLD 1048576

class Connection{
private:
    int _fd;
    int _cgiFd;
    std::string _host;
    std::string _port;
    time_t _time;
    std::string _buffer;
    Request  _request;
    Response _response_obj;
    std::string _response;
    ServerConf _server;
    CGI _cgi;
    bool _done;
    bool _responseDone;
    bool _isChunkedResponse;
    std::string _currentChunk;

public:
    ~Connection();
    Connection(int, ServerConf&, const std::string&, const std::string&);
    Connection(const Connection&);
    int getFd() const;
    int getCgiFd() const;
    ServerConf getServer(); 
    time_t getTime() const;
    bool readRequest();
    bool isDone();
    bool cgiDone();
    bool isCGI() const;
    void readCGIOutput();
    bool isResponseDone();
    bool writeResponse();
    bool setNonBlocking();
    void printRequest();
    std::string setCGIHeaders();
    void updateTimout();
    std::string to_str(int);
    void requestInfo(const std::string&, const std::string&, int, const std::string&, const std::string&, const std::string&);
    void getRquestType();
    void sendGetResponse(Request    &request, ServerConf &server);
    void sendPostResponse(Request   &request, int status_code, ServerConf &server);
    void sendDeleteResponse(Request &request, ServerConf &server);
    void sendErrorPage(Request &request, int code, ServerConf &server);
    std::string generateDirectoryListing(const std::string&, const std::string&);
    std::string getConnectionHeader(Request &request);
    std::string checkForRedirect(Request &request, ServerConf &server);
    std::string sendRedirectResponse(Request &request, const std::string &redirect_url, ServerConf &server);
};

#endif
