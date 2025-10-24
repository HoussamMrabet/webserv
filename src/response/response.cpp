#include "Response.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <ctime>

Response::Response() : _statusCode(200), _statusMessage("OK") {
    setHeader("Server", "webserv/1.0");
}

Response::Response(int code, const std::string& message) 
    : _statusCode(code), _statusMessage(message) {
    setHeader("Server", "webserv/1.0");
    sendError(code);
}

Response::~Response() {}

void Response::setStatus(int code, const std::string& message) {
    _statusCode = code;
    _statusMessage = message;
}

void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void Response::setBody(const std::string& body) {
    _body = body;
    std::ostringstream oss;
    oss << body.size();
    setHeader("Content-Length", oss.str());
}

std::string Response::getStatusMessage(int code) const {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 504: return "Gateway Timeout";
        default: return "Unknown";
    }
}

void Response::handle(const Request& req, const ServerConf& config) {
// /***************************** session *************************************/
// // std::cout << "**************************** handle get\n";
//     std::string link = req.getUri();
//     std::string new_uri = "";
//     if (link == "/profile/login.html" || link == "/profile" || link == "/profile/profile.html") {
// std::cout << "**************************** handle session\n";
//         setHeader("Set-Cookie", req.getHeader("Set-Cookie"));
//         if (Request::loggedIn) {
//             setHeader("X-User-Username", Request::loggedInUser.username);
//             setHeader("X-User-Email", Request::loggedInUser.email);
//             setHeader("X-User-FullName", Request::loggedInUser.fullName);
//             setHeader("X-User-Avatar", Request::loggedInUser.avatar);
//             setHeader("X-User-Job", Request::loggedInUser.job);
//         }
//     }
//     if ((link == "/profile/profile.html" ) && Request::loggedIn == false)
//         new_uri = "/profile/login.html";
//     if ((link == "/profile" || link == "/profile/"||link == "/profile/login.html") && Request::loggedIn == true)
//         new_uri = "/profile/profile.html";
//     if (link == "/logout")
//         new_uri = "/profile";
//     if (!new_uri.empty())
//     {
//         // // response_obj.setStatus(301);
//         // setHeader("Location", redirect_url);
//         // setHeader("Connection", connection_header);
//         // setHeader("Content-Type", "text/html");

//         setStatus(301, "Redirected");
//         setHeader("Location", link);
//         setBody("<html><body><h1>301 redirected</h1></body></html>");
//         setHeader("Content-Type", "text/html");
//     }
// /***************************************************************************/
    std::string method = req.getStrMethod();
    std::cout << "Request method = " << method << std::endl;
    if (method == "GET"/* || method == "HEAD"*/) {
        handleGET(req, config);
    } else if (method == "POST") {
        handlePOST(req, config);
    } else if (method == "DELETE") {
        handleDELETE(req, config);
    } else {
        sendError(405);
    }
    
    // Set connection header
    if (req.getHeader("Connection") == "keep-alive") {
        setHeader("Connection", "keep-alive");
    } else {
        setHeader("Connection", "close");
    }
}

void Response::handleGET(const Request& req, const ServerConf& config) {
    // std::string path = req.getRoot() + req.getPath();
    std::string path = req.getFullPath();
    
    std::cout << "path = " << path << std::endl;
    // Check if file exists
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        sendError(404);
        return;
    }
    
    // Check if it's a directory
    if (S_ISDIR(st.st_mode)) {
        std::cout << "---> directory listing\n";
        // // Try index files
        // const std::vector<std::string>& indexes = config.getIndex();
        // bool found = false;
        
        // for (size_t i = 0; i < indexes.size(); i++) {
        //     std::string indexPath = path;
        //     if (indexPath[indexPath.length() - 1] != '/')
        //         indexPath += "/";
        //     indexPath += indexes[i];
            
        //     if (stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
        //         sendFile(indexPath);
        //         found = true;
        //         break;
        //     }
        // }
        
        // if (!found) {
            // sendDirectory(path, req.getPath(), config.getAutoIndex());

        std::map<std::string, LocationConf> locations = config.getLocations();

            bool auto_index = false;
            std::string location = req.getLocation();
            // std::cout << "location = " << req.getLocation() << std::endl;
            if (locations.find(location) != locations.end())
                auto_index = locations.at(location).getAutoIndex();
            //     index_files = locations.at(location).getIndex();

            sendDirectory(path, req.getUri(), auto_index);
        // }
    } else if (S_ISREG(st.st_mode)) {
        std::cout << "---> regular file\n";
        sendFile(path);
    } else {
        std::cout << "---> wrong file\n";
        sendError(403);
    }
}

void Response::handlePOST(const Request& req, const ServerConf& config) {
    (void)config;
    std::string path = req.getFullPath();
    std::cout << "------ fullPath = " << path << std::endl;
    std::string body = req.getBody();

    // Check body size limit
    std::cout << "body size = " << body.size() 
              << " max = " << req.getBodySizeLimit() << std::endl;
    if (body.size() > req.getBodySizeLimit()) {
        sendError(413);  // Payload Too Large
        return;
    }

    std::string contentType = req.getHeader("content-type");

    if (contentType.find("multipart/form-data") != std::string::npos) {
        std::vector<Multipart *> parts = req.getMultipartData();
        for (size_t i = 0; i < parts.size(); ++i) {
            Multipart *part = parts[i];
            std::string filename = part->getFileName();
            std::string partData = part->getData();

            if (!filename.empty()) {
                std::ofstream file((req.getRoot() + "/" + filename).c_str(), std::ios::binary);
                if (!file) {
                    sendError(403); // Forbidden
                    return;
                }
                file << partData;
                file.close();
            }
        }
    } else {
        // Simple body upload
        std::ofstream file(path.c_str(), std::ios::binary);
        if (!file) {
            sendError(403);
            return;
        }
        file << body;
        file.close();
    }

    // Set response
    setStatus(201, "Created");
    setHeader("Location", req.getUri());
    setBody("<html><body><h1>201 Created</h1></body></html>");
    setHeader("Content-Type", "text/html");
}


void Response::handleDELETE(const Request& req, const ServerConf& ) {
    // std::string path = config.getRoot() + req.getPath();
    std::string path = req.getFullPath();
    
    if (unlink(path.c_str()) == 0) {
        setStatus(204, "No Content");
        setBody("");
    } else {
        sendError(404);
    }
}

void Response::sendError(int code) {
    setStatus(code, getStatusMessage(code));
    
    std::ostringstream html;
    html << "<html><head><title>" << code << " " << _statusMessage 
         << "</title></head><body><center><h1>" << code << " " 
         << _statusMessage << "</h1></center><hr><center>webserv/1.0</center></body></html>";
    
    setBody(html.str());
    setHeader("Content-Type", "text/html");
}

void Response::sendFile(const std::string& path) {
    std::string content = readFile(path);
    if (content.empty()) {
        sendError(500);
        return;
    }
    
    setStatus(200, "OK");
    setBody(content);
    setHeader("Content-Type", getContentType(path));
    // std::cout << "########## contentType = " << getContentType(path) << std::endl;
}

void Response::sendDirectory(const std::string& path, const std::string& uri, bool autoindex) {
    std::cout << "autoindex = " << autoindex << std::endl;
    if (!autoindex) {
        sendError(403);
        return;
    }
    
    std::string listing = generateDirectoryListing(path, uri);
    if (listing.empty()) {
        sendError(500);
        return;
    }
    
    setStatus(200, "OK");
    setBody(listing);
    setHeader("Content-Type", "text/html");
}

std::string Response::readFile(const std::string& path) const {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return "";
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::string Response::getContentType(const std::string& path) const {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
    
    std::string ext = path.substr(dot);
    
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".xml") return "application/xml";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".pdf") return "application/pdf";
    if (ext == ".txt") return "text/plain";
    if (ext == ".mp4") return "video/mp4";
    if (ext == ".mp3") return "audio/mpeg";
    
    return "application/octet-stream";
}

std::string Response::generateDirectoryListing(const std::string& path, 
                                               const std::string& uri) const {
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return "";
    
    std::ostringstream html;
    html << "<html><head><title>Index of " << uri 
         << "</title></head><body><h1>Index of " << uri << "</h1><hr><pre>";
    
    std::string cleanUri = uri;
    if (cleanUri[cleanUri.length() - 1] != '/')
        cleanUri += "/";
    
    // Add parent directory link
    if (uri != "/") {
        html << "<a href=\"..\">../</a>\n";
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        
        std::string fullPath = path;
        if (fullPath[fullPath.length() - 1] != '/')
            fullPath += "/";
        fullPath += name;
        
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                html << "<a href=\"" << cleanUri << name << "/\">" 
                     << name << "/</a>\n";
            } else {
                html << "<a href=\"" << cleanUri << name << "\">" 
                     << name << "</a>\n";
            }
        }
    }
    
    closedir(dir);
    html << "</pre><hr></body></html>";
    return html.str();
}

void Response::fromCGI(const std::string& cgiOutput) {
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        headerEnd = cgiOutput.find("\n\n");
/*********************************************************************************/
/* if we want to be restrict about having py output header in the file keep this part*/
        // if (headerEnd == std::string::npos) {
        //     sendError(502);
        //     return;
        // }
/* if we want to accept file even without py header and just add it keep this part*/
// this is a non stardard behaviour!!
        if (headerEnd == std::string::npos) {
            // No header section found — assume pure body content
            setStatus(200, "OK");
            setHeader("Content-Type", "text/html");
            setBody(cgiOutput);
            return;
        }
/*********************************************************************************/
    }
    
//------------------------------------------------------------------------------//
// separator can be either \n\n (py) or  \r\n\r\n

// this par take into account just \r\n\r\n
    // std::string headerSection = cgiOutput.substr(0, headerEnd);
    // std::string bodySection = cgiOutput.substr(headerEnd + 4);

// and this part handles both cases
    std::string headerSection = cgiOutput.substr(0, headerEnd);
    std::string bodySection;

    // Detect which separator we actually found
    if (cgiOutput.compare(headerEnd, 4, "\r\n\r\n") == 0)
        bodySection = cgiOutput.substr(headerEnd + 4);
    else
        bodySection = cgiOutput.substr(headerEnd + 2);

//------------------------------------------------------------------------------//

// std::cerr << "[CGI OUTPUT START]\n" << cgiOutput << "\n[CGI OUTPUT END]\n";
// std::cerr << "HeaderEnd=" << headerEnd << " BodySize=" << bodySection.size() << "\n";

    
    // Parse headers
    std::istringstream iss(headerSection);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.empty() || line == "\r")
            continue;
        
        // Remove \r if present
        if (!line.empty() && line[line.length() - 1] == '\r')
            line = line.substr(0, line.length() - 1);
        
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            
            // Trim leading space from value
            if (!value.empty() && value[0] == ' ')
                value = value.substr(1);
            
            // Special handling for Status header
            if (key == "Status") {
                std::istringstream statusStream(value);
                int code;
                statusStream >> code;
                std::string message;
                std::getline(statusStream, message);
                if (!message.empty() && message[0] == ' ')
                    message = message.substr(1);
                setStatus(code, message);
            } else {
                setHeader(key, value);
            }
        }
    }
    
    // Set default status if not provided
    if (_statusCode == 0)
        setStatus(200, "OK");
    
    setBody(bodySection);
    setHeader("Server", "webserv/1.0");
}

std::string Response::build() const {
    std::ostringstream oss;
    
    // Status line
    oss << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    
    // Empty line
    oss << "\r\n";
    
    // Body
    oss << _body;
    
    return oss.str();
}