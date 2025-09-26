#include "Response2.hpp"
#include <sstream>
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include <algorithm>

// Helper function from your original code
std::string to_string(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

void Response2::handle(const Request& req, ServerConf& config) {
    // Check for redirects first
    std::string redirect_url = checkRedirect(req, config);
    if (!redirect_url.empty()) {
        setRedirect(301, redirect_url);
        return;
    }
    
    // Handle different methods
    std::string method = req.getStrMethod();
    if (method == "GET") {
        handleGet(req, config);
    } else if (method == "POST") {
        handlePost(req, config);
    } else if (method == "DELETE") {
        setError(501, "Not Implemented");
    } else {
        setError(405, "Method Not Allowed");
    }
}

void Response2::fromCGI(const std::string& cgi_output) {
    // Parse CGI output - adapted from your parseOutput function
    size_t header_end = cgi_output.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        header_end = cgi_output.find("\n\n");
        if (header_end != std::string::npos) {
            header_end += 2;
        }
    } else {
        header_end += 4;
    }
    
    if (header_end != std::string::npos) {
        std::string headers_part = cgi_output.substr(0, header_end);
        body = cgi_output.substr(header_end);
        
        // Check if CGI already included HTTP status line
        if (headers_part.find("HTTP/") == 0) {
            // CGI provided full response2, use as-is
            raw_response2 = cgi_output;
            return;
        } else {
            // Parse CGI headers
            parseCGIHeaders(headers_part);
        }
    } else {
        // No headers, just body
        body = cgi_output;
        setHeader("Content-Type", "text/html");
    }
    
    status_code = 200;
    setHeader("Content-Length", to_string(body.length()));
}

std::string Response2::build() const {
    if (!raw_response2.empty()) {
        return raw_response2;  // CGI provided full response2
    }
    
    std::ostringstream response2;
    
    // Status line
    response2 << "HTTP/1.1 " << status_code << " " << getStatusText() << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        response2 << it->first << ": " << it->second << "\r\n";
    }
    
    // End of headers
    response2 << "\r\n";
    
    // Body
    response2 << body;
    
    return response2.str();
}

// Private helper methods
void Response2::handleGet(const Request& req, ServerConf& config) {
    std::string uri = req.getUri();
    std::string full_path = "./www" + uri;
    
    struct stat file_stat;
    if (stat(full_path.c_str(), &file_stat) != 0) {
        setError(404, "Not Found");
        return;
    }
    
    if (S_ISREG(file_stat.st_mode)) {
        // Regular file
        serveFile(full_path);
    } else if (S_ISDIR(file_stat.st_mode)) {
        // Directory
        handleDirectory(full_path, uri, config);
    } else {
        setError(403, "Forbidden");
    }
}

void Response2::handlePost(const Request& req, ServerConf& config) {
    (void)config;  // Suppress unused warning
    
    // Your POST logic was already handled in Request parsing
    // Just send success response2
    status_code = 200;
    setHeader("Content-Type", "text/html");
    setHeader("Connection", getConnectionType(req));
    
    body = "<html><body><h1>POST Successful</h1>"
           "<p>Your POST request was processed.</p></body></html>";
    setHeader("Content-Length", to_string(body.length()));
}

void Response2::serveFile(const std::string& file_path) {
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file) {
        setError(403, "Forbidden");
        return;
    }
    
    // Read file content
    std::ostringstream buffer;
    buffer << file.rdbuf();
    body = buffer.str();
    
    status_code = 200;
    setHeader("Content-Type", getContentType(file_path));
    setHeader("Content-Length", to_string(body.length()));
    setHeader("Connection", "close");  // Simple approach
}

void Response2::handleDirectory(const std::string& dir_path, const std::string& uri, ServerConf& config) {
    // Try index files first
    std::vector<std::string> index_files = config.getIndex();
    for (size_t i = 0; i < index_files.size(); ++i) {
        std::string index_path = dir_path;
        if (index_path[index_path.length() - 1] != '/') {
            index_path += "/";
        }
        index_path += index_files[i];
        
        struct stat file_stat;
        if (stat(index_path.c_str(), &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
            serveFile(index_path);
            return;
        }
    }
    
    // No index file, check if auto-index is enabled
    if (config.getAutoIndex()) {
        generateDirectoryListing(dir_path, uri);
    } else {
        setError(403, "Forbidden");
    }
}

void Response2::generateDirectoryListing(const std::string& dir_path, const std::string& uri) {
    DIR* dir = opendir(dir_path.c_str());
    if (!dir) {
        setError(403, "Forbidden");
        return;
    }
    
    std::ostringstream html;
    html << "<!DOCTYPE html>\n<html><head><title>Index of " << uri << "</title></head>\n";
    html << "<body><h1>Index of " << uri << "</h1><hr><pre>\n";
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || (name == ".." && uri == "/")) continue;
        
        html << "<a href=\"";
        if (uri[uri.length() - 1] != '/') html << uri << "/";
        else html << uri;
        html << name << "\">" << name << "</a>\n";
    }
    closedir(dir);
    
    html << "</pre><hr></body></html>";
    
    status_code = 200;
    body = html.str();
    setHeader("Content-Type", "text/html");
    setHeader("Content-Length", to_string(body.length()));
    setHeader("Connection", "close");
}

void Response2::setError(int code, const std::string& message) {
    status_code = code;
    
    // Try to load custom error page
    std::string error_file = "./www/errors/" + to_string(code) + ".html";
    std::ifstream file(error_file.c_str());
    if (file) {
        std::ostringstream buffer;
        buffer << file.rdbuf();
        body = buffer.str();
    } else {
        // Default error page
        body = "<html><body><h1>" + to_string(code) + " " + message + "</h1></body></html>";
    }
    
    setHeader("Content-Type", "text/html");
    setHeader("Content-Length", to_string(body.length()));
    setHeader("Connection", "close");
}

void Response2::setRedirect(int code, const std::string& url) {
    status_code = code;
    setHeader("Location", url);
    setHeader("Connection", "close");
    
    body = "<html><body><h1>" + to_string(code) + " Moved</h1>"
           "<p>Document moved <a href=\"" + url + "\">here</a>.</p></body></html>";
    setHeader("Content-Length", to_string(body.length()));
}

std::string Response2::checkRedirect(const Request& req, ServerConf& config) {
    std::string uri = req.getUri();
    std::map<std::string, LocationConf> locations = config.getLocations();
    
    // Find best matching location
    std::string best_match = "";
    const LocationConf* best_location = NULL;
    
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        if (uri.find(it->first) == 0 && it->first.length() > best_match.length()) {
            best_match = it->first;
            best_location = &(it->second);
        }
    }
    
    if (best_location) {
        return best_location->getRedirectUrl();
    }
    
    return "";
}

void Response2::parseCGIHeaders(const std::string& header_text) {
    std::istringstream stream(header_text);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") break;
        
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            
            // Trim whitespace
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
                value.erase(0, 1);
            }
            while (!value.empty() && (value[value.length()-1] == '\r' || value[value.length()-1] == '\n')) {
                value.erase(value.length()-1);
            }
            
            setHeader(key, value);
        }
    }
}

std::string Response2::getContentType(const std::string& file_path) const {
    size_t dot = file_path.find_last_of('.');
    if (dot == std::string::npos) return "application/octet-stream";
    
    std::string ext = file_path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "text/javascript";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "txt") return "text/plain";
    
    return "application/octet-stream";
}

std::string Response2::getStatusText() const {
    switch (status_code) {
        case 200: return "OK";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        default: return "Unknown";
    }
}

std::string Response2::getConnectionType(const Request& req) const {
    std::string conn = req.getHeader("Connection");
    std::transform(conn.begin(), conn.end(), conn.begin(), ::tolower);
    return (conn == "close") ? "close" : "keep-alive";
}

void Response2::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}