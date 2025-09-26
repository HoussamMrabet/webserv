#ifndef RESPONSE2_HPP
#define RESPONSE2_HPP

#include "Request.hpp"
#include "ServerConf.hpp"
#include <string>
#include <map>

class Request;
class ServerConf;
class LocationConf;

class Response2 {
private:
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string raw_response2;  // For CGI response2s that include full HTTP response2
    
public:
    Response2() : status_code(200) {}
    
    // Main interface methods
    void handle(const Request& req, ServerConf& config);
    void fromCGI(const std::string& cgi_output);
    std::string build() const;
    
private:
    // HTTP method handlers
    void handleGet(const Request& req, ServerConf& config);
    void handlePost(const Request& req, ServerConf& config);
    
    // File serving
    void serveFile(const std::string& file_path);
    void handleDirectory(const std::string& dir_path, const std::string& uri, ServerConf& config);
    void generateDirectoryListing(const std::string& dir_path, const std::string& uri);
    
    // Error and redirect handling
    void setError(int code, const std::string& message);
    void setRedirect(int code, const std::string& url);
    std::string checkRedirect(const Request& req, ServerConf& config);
    
    // CGI support
    void parseCGIHeaders(const std::string& header_text);
    
    // Utility functions
    std::string getContentType(const std::string& file_path) const;
    std::string getStatusText() const;
    std::string getConnectionType(const Request& req) const;
    void setHeader(const std::string& key, const std::string& value);
};

#endif