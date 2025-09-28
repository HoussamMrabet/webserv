#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include <iostream>
#include <sys/socket.h> // accept 
#include <fcntl.h> // fcntl
#include <unistd.h>   // read, write, close
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConf.hpp"
#include "cgi.hpp"

#define LARGE_FILE_THRESHOLD 1048576  // 1MB - files larger than this will use chunked transfer

class Connection{
private:
    int _fd;
    int _cgiFd;
    time_t _time;
    std::string _buffer;
    Request* _request;
    Response _response_obj;  // For chunked responses
    std::string _response;   // For simple responses
    static ServerConf _server;
    bool _done;
    bool _responseDone;
    bool _isChunkedResponse; // Flag to track if we're doing chunked response

public:
    ~Connection();
    Connection(int, ServerConf&);
    Connection(const Connection&);
    int getFd() const;
    int getCgiFd() const;
    ServerConf getServer(); 
    time_t getTime() const;
    bool readRequest();
    bool isDone();
    bool isResponseDone();
    bool writeResponse();
    void requestInfo(int, const std::string&, const std::string&, const std::string&);
    void printRequest(); // to remove
    void setNonBlocking();
    void updateTimout();
    // std::string getRequestMethod();
    void sendGetResponse(Request    &request, ServerConf &server);
    void sendPostResponse(Request   &request, int status_code, ServerConf &server);
    // void sentPostResponse(Request   &request, ServerConf &server);
    // void sendDeleteResponse(Request &request, ServerConf &server);
    // void sendAutoIndex(Response &response_obj, const std::string &full_path, const std::string &requested_path);
    void sendErrorPage(Request &request, int code, ServerConf &server);
    std::string getConnectionHeader(Request &request);
    std::string checkForRedirect(Request &request, ServerConf &server);
    std::string sendRedirectResponse(Request &request, const std::string &redirect_url, ServerConf &server);
    // bool cgiResponse(int (&fd_in)[], int (&fd_out)[]);
};

#endif
