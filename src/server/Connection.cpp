#include "Connection.hpp"
#include <algorithm>
#include <dirent.h>
#include <vector>
#include <sstream>
ServerConf Connection::_server;
// ServerConf Connection::_server = ConfigBuilder::generateServers("config/config.conf").back();
struct stat fileStat;

#define DEFAULT_RESPONSE "HTTP/1.1 200 OK\r\n" \
                         "Content-Length: 10\r\n" \
                         "Content-Type: text/plain\r\n" \
                         "Connection: keep-alive\r\n" \
                         "\r\n" \
                         "Received!!\r\n" // Connection keep-alive but still ends !!!!!

// Helper function to generate directory listing HTML
std::string generateDirectoryListing(const std::string& directory_path, const std::string& request_uri) {
    std::stringstream html;
    
    // HTML header
    html << "<!DOCTYPE html>\n<html>\n<head>\n";
    html << "<title>Index of " << request_uri << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "h1 { border-bottom: 1px solid #ccc; }\n";
    html << "table { border-collapse: collapse; width: 100%; }\n";
    html << "th, td { text-align: left; padding: 8px 12px; border-bottom: 1px solid #ddd; }\n";
    html << "th { background-color: #f2f2f2; }\n";
    html << "tr:hover { background-color: #f5f5f5; }\n";
    html << "a { text-decoration: none; color: #0066cc; }\n";
    html << "a:hover { text-decoration: underline; }\n";
    html << ".dir { font-weight: bold; }\n";
    html << "</style>\n</head>\n<body>\n";
    html << "<h1>Index of " << request_uri << "</h1>\n";
    
    // Open directory
    DIR* dir = opendir(directory_path.c_str());
    if (dir == NULL) {
        html << "<p>Error: Cannot read directory</p>\n";
        html << "</body>\n</html>";
        return html.str();
    }
    
    // Create vector to store directory entries for sorting
    std::vector<std::pair<std::string, bool> > entries; // name, is_directory
    struct dirent* entry;
    struct stat entry_stat;
    
    while ((entry = readdir(dir)) != NULL) {
        std::string entry_name = entry->d_name;
        
        // Skip hidden files starting with '.' except for parent directory
        if (entry_name[0] == '.' && entry_name != "..") {
            continue;
        }
        
        // Get full path for stat
        std::string full_entry_path = directory_path + "/" + entry_name;
        bool is_directory = false;
        
        if (stat(full_entry_path.c_str(), &entry_stat) == 0) {
            is_directory = S_ISDIR(entry_stat.st_mode);
        }
        
        entries.push_back(std::make_pair(entry_name, is_directory));
    }
    closedir(dir);
    
    // Sort entries: directories first, then files, both alphabetically  
    // Simple bubble sort since we need C++98 compatibility
    for (size_t i = 0; i < entries.size(); i++) {
        for (size_t j = i + 1; j < entries.size(); j++) {
            bool should_swap = false;
            
            // Directories come before files
            if (entries[i].second != entries[j].second) {
                should_swap = entries[j].second; // j is directory, i is file
            } else {
                // Same type, sort alphabetically
                should_swap = entries[i].first > entries[j].first;
            }
            
            if (should_swap) {
                std::pair<std::string, bool> temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }
    
    // Generate table
    html << "<table>\n";
    html << "<tr><th>Name</th><th>Type</th><th>Size</th></tr>\n";
    
    for (std::vector<std::pair<std::string, bool> >::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        const std::string& entry_name = it->first;
        bool is_directory = it->second;
        
        html << "<tr>";
        
        // Name column with link
        std::string href = request_uri;
        if (href[href.length() - 1] != '/') {
            href += "/";
        }
        href += entry_name;
        
        html << "<td><a href=\"" << href << "\"";
        if (is_directory) {
            html << " class=\"dir\"";
        }
        html << ">" << entry_name;
        if (is_directory) {
            html << "/";
        }
        html << "</a></td>";
        
        // Type column
        html << "<td>" << (is_directory ? "Directory" : "File") << "</td>";
        
        // Size column
        std::string full_entry_path = directory_path + "/" + entry_name;
        struct stat entry_stat;
        if (stat(full_entry_path.c_str(), &entry_stat) == 0 && !is_directory) {
            html << "<td>" << entry_stat.st_size << " bytes</td>";
        } else {
            html << "<td>-</td>";
        }
        
        html << "</tr>\n";
    }
    
    html << "</table>\n";
    html << "</body>\n</html>";
    
    return html.str();
}

Connection::~Connection(){ 
    CHOROUK && std::cout << "***********  connection destructor called!!! ***********\n";
    // delete _request;
}

void Connection::requestInfo(const std::string& host, const std::string& port, int status, const std::string& method, const std::string& path, const std::string& version) {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "[%a %b %d %H:%M:%S %Y]", t);
    // std::cout << M"" << buf << " [" << _server.getListen().begin()->first
            //   << "]:" << _server.getListen().begin()->second  << " [" << status << "]: "
    std::cout << M"" << buf << " [" << host << "]:" << port  << " [" << status << "]: "
              << method << " " << path << " " << version << B"" << std::endl;
}

Connection::Connection(int fd, ServerConf& server, const std::string& host, const std::string& port): _fd(-1),
                                                    _host(host), _port(port),
                                                    _time(time(NULL)),
                                                    _done(false)
                                                    // _responseDone(false),
                                                    // _isChunkedResponse(false)/*....*/ 
                                                    {
    _fd = accept(fd, NULL, NULL);
    _server = server;
    // _request = new Request();
    _cgiFd = -1;
    // std::cout << "Connection constructor fd " << _fd << "\n";
    // std::cout << _server.getRoot() << std::endl;
    if (_fd == -1) {
        perror("Accept failed");
        // exit(1);/// check !!
    }
    setNonBlocking();
    // std::cout << "New connection (fd " << _fd << ")\n" << std::endl;
    // std::cout << "[Sun Sep 14 15:23:11 2025] [::1]:52214 [302]: " << _scriptFileName << "\n" << std::endl;
    // std::cout << "[Sun Sep 14 15:23:11 2025] [::1]:52214 [302]: ";
}

Connection::Connection(const Connection& connection){
    _fd = connection._fd;
    _time = connection._time;
    _buffer = connection._buffer;
    // _request = connection._request;
    _done = connection._done;
    // _responseDone = connection._responseDone;
    // _isChunkedResponse = connection._isChunkedResponse;
}

int Connection::getFd() const{ return (_fd);}

bool Connection::readRequest(){
    char buffer[1024] = {0};
    ssize_t bytesRead = 0;

    CHOROUK && std::cout << M"" << _request.getUri() << B"\n";

    if  (!_request.isDone())
    {
        CHOROUK && std::cout << "------- read fd = " << _fd << std::endl;
        bytesRead = read(_fd, buffer, sizeof(buffer));
        CHOROUK && std::cout << "------- bytes = " << bytesRead << std::endl;
        if (bytesRead > 0)
        {
            _buffer.append(buffer, bytesRead);
            _request.parseRequest(_buffer);
            _buffer.clear();
            updateTimout();  // update activity timestamp
            // else continue;
        }
        else
        {
            _request.parseRequest();
            // _buffer.clear();
            // std::cout << "Client disconnected!" << std::endl;
            // _done = true;
            // _responseDone = true;
            // return (false);
        }
        // if (bytesRead < 1024){
        //     _done = true;
        //     return (true);
        // } 
        // else
        // {
        //     std::cout << "Error reading from socket" << std::endl;
        //     // check for errno!!!
        //     perror("Read failed");
        //     return (false);
        // }
    }
    _done = _request.isDone();
    // CHOROUK && _request.printRequest();


    // if (_request.isDone()) // remove from here and add to webserv in order to add pipe_fds to pollfds
    //     _response = CGI::executeCGI(_request);
    return (true);
}

bool Connection::writeResponse(){ // check if cgi or not, if cgi call cgiResponse!!!
    // First, check if we're in the middle of sending a chunked response

    CHOROUK && std::cout << "------- write fd = " << _fd << std::endl;
    // std::cout << M"" << _request.getUri() << B"\n";
    requestInfo(_host, _port,
                _request.getStatusCode(), \
                _request.getStrMethod(), \
                _request.getUri(), \
                _request.getHeader("httpVersion"));
    // if (_isChunkedResponse) {
    if (_response.isChunked()) {
        // Continue sending chunks from existing response
        if (!_response.isFinished()) {
            std::string chunk = _response.getResponseChunk();
            if (!chunk.empty()) {
                ssize_t bytes_sent = write(_fd, chunk.c_str(), chunk.length());
                if (bytes_sent == -1) {
                    perror("Chunked write failed");
                    return (false);
                } else if (bytes_sent == 0) {
                    MOHAMED && std::cout << "Connection closed by peer during chunked transfer" << std::endl;
                    return (false);
                } else if (bytes_sent < (ssize_t)chunk.length()) {
                    // Partial write - handle this case
                    MOHAMED && std::cout << "Partial write: " << bytes_sent << "/" << chunk.length() << " bytes" << std::endl;
                    return (false);
                }
                MOHAMED && std::cout << "Sent chunk of size: " << bytes_sent << std::endl;
                return (true); // Successfully sent a chunk, return
            }
        } else {
            // All chunks sent, mark response as done
            MOHAMED && std::cout << "Chunked response complete" << std::endl;
            _response.setAsDone();
            // _responseDone = true;
            _response.setAsChunked(false);
            // _isChunkedResponse = false;
            return (true);
        }
    }
    
    // Regular response processing - only if not in chunked mode
    // Check for redirects first
    std::string redirect_url = checkForRedirect(_request, _server);
    // CHOROUK && std::cout << C"--------- REDIRECTION FOUND!!!! -----------" << B"\n";
    if (!redirect_url.empty()) {
        sendRedirectResponse(_request, redirect_url, _server);
        // MOHAMED && std::cout << "Redirect Response:\n" << _response << std::endl;
        updateTimout();
    }
    // else if (_request.isCGI())
    // {
    //     CHOROUK && std:: cout << M"IT IS CGI!!!!!\n";
    //     _response = CGI::executeCGI(_request, _server);
    //     // _cgiFd = CGI::getFd();
    //     updateTimout();
    // }
    else if ( _request.getStatusCode() != 200){
        MOHAMED && std::cout << "Error status code: " << _request.getStatusCode()  << std::endl;
        MOHAMED && std::cout << "Error message: " << _request.getMessage()  << std::endl;
        sendErrorPage(_request, _request.getStatusCode(), _server);
        // MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "POST"){
        // sentPostResponse(_request, _server);
        sendPostResponse(_request, _request.getStatusCode(), _server);
        // MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "DELETE"){
        // sendDeleteResponse(_request, _server);
        sendErrorPage(_request, 501, _server); // not implemented
        // MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "GET"){ // can use pointer to member function 
        sendGetResponse(_request, _server);
        // if (!_isChunkedResponse) {
        if (!_response.isChunked()) {
            // MOHAMED && std::cout << _response << std::endl;
        }
        updateTimout();
    }
    else{ // to check!!!!
        sendErrorPage(_request, 501, _server); // not implemented
        updateTimout();
    }
    // // Handle regular (non-chunked) response sending
    // // Only executed if we're not in chunked mode or if chunked response just started
    // if (_response.empty()){
    //     _response = DEFAULT_RESPONSE;
    //     updateTimout();
    // }
    // if (!_isChunkedResponse) {
    if (!_response.isChunked()) {
        CHOROUK && std::cout << "writing on fd = " << _fd << std::endl;
        int b = write(_fd, _response.getResponse().c_str(), _response.getResponse().length());
        // int b = write(_fd, _response.c_str(), _response.length());
        if (b == -1){
            // perror("Write failed"); // throw
            return (false);
        }
        CHOROUK && std::cout << "----------- STILL HERE ---------------\n";
        updateTimout();
        // Response is complete
        _response.setAsDone();
        // _responseDone = true;
    }
    
    return (true);
}

// std::string Connection::getRequestMethod(){
//     t_method method = _request.getMethod();
//     switch (method)
//     {
//         case GET:
//             return "GET";
//         case POST:
//             return "POST";
//         case DELETE:
//             return "DELETE";
//         default:
//             return "UNDEFINED";
//     }
// }

std::string to_str(int n){
    std::stringstream ss; ss << n;
    return (ss.str());
}

int Connection::getCgiFd() const{ return (_cgiFd);}

bool Connection::isCGIRequest(const std::string full_path){
    if (_request.isCGI())
        return (true);
    if (full_path.length() >= 4 && full_path.substr(full_path.length() - 4) == ".php"){
        _request.setCgiType(".php");
        return (true);
    }
    else if (full_path.length() >= 3 && full_path.substr(full_path.length() - 3) == ".py"){
        _request.setCgiType(".py");
        return (true);
    }
    return (false);
    // if ((lastPart.empty() || (lastPart == "cgi-bin")) && \
    //         (this->uri == "/cgi-bin/" || this->uri == "/cgi-bin") /* && auto_index == true*/){

    //             this->cgiType = ".py"; // default cgi file
    // //         // check if cgi index file exists in location 
    // //         // if first doesn't exist check for second or global index file
    // //         if (!indexs.empty()){
    // //             this->uriFileName = indexs[0];
    // //         }
    // //         else if (!server.getIndex().empty()){
    // //             this->uriFileName = server.getIndex()[0]; // global index file
    // //         }
    // //         else {
    // //             this->uriFileName = ""; // default cgi file
    // //         }
    //         // if (this->uri[this->uri.length() - 1] != '/')
    //         //     this->uri += "/";
    //         // this->uri +=  this->uriFileName;
    //         // std::cout << "++++ uri after adding index: " << this->uri << std::endl;
    // }
}

void Connection::sendErrorPage(Request &request, int code, ServerConf &server){
    // Response response_obj;
    std::string error_page;
    std::map<std::string, std::string> error_pages = server.getErrorPages();
    std::string connection_header = getConnectionHeader(request);
    
    // Check if there's a custom error page defined in config
    if (error_pages.find(to_str(code)) != error_pages.end()) {
        error_page = error_pages[to_str(code)];
        // Attach ./www/errors/ to the error page path
        std::string full_error_path = "./www/errors/" + error_page;
        if (_response.fileExists(full_error_path)) {
            _response.setStatus(code);
            _response.setBodyFromFile(full_error_path);
            _response.setHeader("Content-Type", _response.getContentType(full_error_path));
            _response.setHeader("Connection", connection_header);
            _response.buildResponse();
            _response.setAsChunked(false);
            // _isChunkedResponse = false;
            return;
        }
    }
    
    // Fallback: check for standard error pages in ./www/errors/
    std::string standard_error_page = "./www/errors/" + to_str(code) + ".html";
    if (_response.fileExists(standard_error_page)) {
        _response.setStatus(code);
        _response.setBodyFromFile(standard_error_page);
        _response.setHeader("Content-Type", _response.getContentType(standard_error_page));
        _response.setHeader("Connection", connection_header);
        _response.buildResponse();
        _response.setAsChunked(false);
        // _isChunkedResponse = false;
        return;
    }
    
    // Default error message if no custom page is found or file doesn't exist
    _response.setStatus(code);
    _response.setBody("<html><body><h1>" + to_str(code) + " " + _response.getStatusMessage(code) + "</h1></body></html>");
    _response.setHeader("Content-Type", "text/html");
    _response.setHeader("Connection", connection_header);
    _response.buildResponse();
    _response.setAsChunked(false);
    // _isChunkedResponse = false;
}

void Connection::sendGetResponse(Request &request  , ServerConf &server){
    // Create a Response object and build it step by step
    // Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    if (_request.getUri() == "/profile/login.html" || _request.getUri() == "/profile" || _request.getUri() == "/profile/profile.html") {
        _response.setHeader("Set-Cookie", _request.getHeader("Set-Cookie"));
        if (Request::loggedIn) {
            _response.setHeader("X-User-Username", Request::loggedInUser.username);
            _response.setHeader("X-User-Email", Request::loggedInUser.email);
            _response.setHeader("X-User-FullName", Request::loggedInUser.fullName);
            _response.setHeader("X-User-Avatar", Request::loggedInUser.avatar);
            _response.setHeader("X-User-Job", Request::loggedInUser.job);
        }
    }
    std::string requested_path = request.getUri();
    std::string document_root;
    std::string full_path;
    
    // Find the matching location configuration
    std::string location_path = "/";
    std::map<std::string, LocationConf> locations = server.getLocations();
    
    // Find the best matching location (longest prefix match)
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); 
         it != locations.end(); ++it) {
        if (requested_path.find(it->first) == 0 && it->first.length() > location_path.length()) {
            location_path = it->first;
        }
    } // what if location is not found!!
    if (locations.find(location_path) == locations.end()) {
        // No matching location found, use default root
        document_root = server.getRoot();
        full_path = "." + document_root + requested_path;
    }
    else{
        document_root = locations[location_path].getRoot();
        if (document_root.empty())
            document_root = server.getRoot();
        std::string file_name = request.getUriFileName();
        full_path = "." + document_root + "/" + file_name;
        if (file_name.empty()) {
            // bool auto_index = locations.at(location_path).getAutoIndex();
            // if (auto_index){
                std::vector<std::string> indexes = locations[location_path].getIndex();
                if (indexes.empty()) {
                    indexes = server.getIndex();
                }
                else {
                    for (std::vector<std::string>::const_iterator it = indexes.begin(); 
                        it != indexes.end(); ++it) {
                        std::string index_path = "." + document_root;
                        if (index_path[index_path.length() - 1] != '/') {
                            index_path += "/";
                        }
                        index_path += *it;
                        if (stat(index_path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                            full_path = index_path;
                            break;
                        }
                    }
                }
            // }
            // else should i check for directory listing
            // or return error page?
        }

    }
    // Always use ./www as the document root
    // document_root = "./www";
    // MOHAMED && std::cout << "Using document root: " << document_root << std::endl; 
    // MOHAMED && std::cout << "is it a directory??: " << S_ISDIR(fileStat.st_mode) << std::endl; 
    
    // Construct full path - handle the case where requested_path starts with '/'
    // if empty then it is a directory request, no need to check here
    
    // if (requested_path[0] == '/') {
    //     full_path = document_root + requested_path;
    // } else {
    //     full_path = document_root + "/" + requested_path;
    // }
    
    MOHAMED && std::cout << "Requested path: " << requested_path << std::endl;
    MOHAMED && std::cout << "Document root: " << document_root << std::endl;
    MOHAMED && std::cout << "Full path: " << full_path << std::endl;
    MOHAMED && std::cout << "*/*/*/*/ " << (stat(full_path.c_str(), &fileStat) == 0) << std::endl;
    if (stat(full_path.c_str(), &fileStat) == 0) {
        // File exists
        if (S_ISREG(fileStat.st_mode)) {
            // Check file size to decide between regular and chunked response
            size_t file_size = static_cast<size_t>(fileStat.st_size);
            MOHAMED && std::cout << "File size: " << file_size << " bytes" << std::endl;
            
            if (file_size > LARGE_FILE_THRESHOLD) {
                // Use chunked transfer for large files
                MOHAMED && std::cout << "Using chunked transfer for large file" << std::endl;
                _response.prepareResponse(full_path);
                _response.setAsChunked(true);
                // _isChunkedResponse = true;  
                // _response = ""; // Clear regular response since we're using chunked
                return;
            } else {
                // Use regular response for small files
                MOHAMED && std::cout << "Using regular response for small file" << std::endl;
                /*************************/
                _response.setStatus(200);
                // if (_request.isCGI()){
                if (isCGIRequest(full_path)){
                    std::string res = CGI::executeCGI(_request, _server, full_path);
                    _response.parseCgiOutput(res);
                }
                else {
                    _response.setBodyFromFile(full_path);
                }
                /**********************************/
                _response.setHeader("Connection", connection_header);
                _response.buildResponse();
                _response.setAsChunked(false);
                // _isChunkedResponse = false;
                return;
            }
        } else if (S_ISDIR(fileStat.st_mode)) {
            MOHAMED && std::cout << "*/*/*/*/   It's a directory." << std::endl;
            // bool auto_index = false;
            // std::vector<std::string> index_files;
            
            // if (locations.find(location_path) != locations.end()) {
            //     auto_index = locations.at(location_path).getAutoIndex();
            //     index_files = locations.at(location_path).getIndex();
            // }
            
            // if (index_files.empty()) {
            //     index_files = server.getIndex();
            // }
            
            // // Try to serve index files
            // bool index_found = false;
            // for (std::vector<std::string>::const_iterator it = index_files.begin(); 
            //      it != index_files.end(); ++it) {
            //     std::string index_path = full_path;
            //     if (index_path[index_path.length() - 1] != '/') {
            //         index_path += "/";
            //     }
            //     index_path += *it;
                
                // if (stat(index_path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                //     // Check file size to decide between regular and chunked response
                //     size_t file_size = static_cast<size_t>(fileStat.st_size);
                //     MOHAMED && std::cout << "Index file size: " << file_size << " bytes" << std::endl;
                    
                //     if (file_size > LARGE_FILE_THRESHOLD) {
                //         // Use chunked transfer for large index files
                //         MOHAMED && std::cout << "Using chunked transfer for large index file" << std::endl;
                //         _response.prepareResponse(index_path);
                //         _isChunkedResponse = true;
                //         _response = ""; // Clear regular response since we're using chunked
                //         return;
                //     } else {
                        // // Use regular response for small index files
                        // MOHAMED && std::cout << "Using regular response for small index file" << std::endl;
                        // /*************************/
                        // _response.setStatus(200);
                        // MOHAMED && std::cout << "Index path: " << index_path << std::endl;
                        // if (_request.isCGI()){
                        // // if (_request.isCGI()){
                        //     _response = CGI::executeCGI(_request, _server);
                        //     _response.parseCgiOutput(_response);
                        // }
                        // else {
                        //     _response.setBodyFromFile(full_path);
                        // }
            //             /**********************************/
            //             _response.setHeader("Connection", connection_header);
            //             _response = _response.buildResponse();
            //             _isChunkedResponse = false;
            //             return;
            //         }
            //     }
            // }
            bool auto_index = locations.at(location_path).getAutoIndex();
            // if (!index_found) {
                if (auto_index) {
                    // Generate directory listing
                    std::string directory_listing = generateDirectoryListing(full_path, requested_path);
                    _response.setStatus(200);
                    _response.setHeader("Content-Type", "text/html");
                    _response.setHeader("Connection", connection_header);
                    _response.setBody(directory_listing);
                    _response.buildResponse();
                    _response.setAsChunked(false);
                    // _isChunkedResponse = false;
                    return;
                } else {
                    // Directory listing forbidden
                    _response.setStatus(403);
                    _response.setBody("Forbidden - Directory listing disabled");
                    _response.setHeader("Content-Type", "text/html");
                    _response.setHeader("Connection", connection_header);
                    _response.buildResponse();
                    _response.setAsChunked(false);
                    // _isChunkedResponse = false;
                    return;
                }
            // }
            // MOHAMED && std::cout << "It's a directory." << std::endl;
        } else {
            MOHAMED && std::cout << "It's neither a regular file nor a directory." << std::endl;
            _response.setStatus(403);
            _response.setBody("Forbidden");
            _response.setHeader("Content-Type", "text/html");
            _response.setHeader("Connection", connection_header);
            _response.buildResponse();
            _response.setAsChunked(false);
            // _isChunkedResponse = false;
            return;
        }
    } else {
        // File does not exist
        MOHAMED && std::cout << "File does not exist: " << full_path << std::endl;
        _response.setStatus(404);
        _response.setBody("File not found");
        _response.setHeader("Content-Type", "text/html");
        _response.setHeader("Connection", connection_header);
        _response.buildResponse();
        _response.setAsChunked(false);
        // _isChunkedResponse = false;
        return;
    }   
    
    // This should not be reached now since we handle all cases above
    _response.buildResponse();
    _response.setAsChunked(false);
    // _isChunkedResponse = false;
}

void Connection::sendPostResponse(Request &request, int status_code, ServerConf &server) {
    // Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    (void)server; // Suppress unused parameter warning
    
    // Since POST logic is already handled in Request class, just send success response
    _response.setStatus(status_code);
    _response.setHeader("Content-Type", "text/html");
    _response.setHeader("Connection", connection_header);
    
    std::string success_body = "<html><body><h1>POST Request Successful</h1>"
                              "<p>Your POST request has been processed successfully.</p>"
                              "</body></html>";
    
    _response.setBody(success_body);
    _response.buildResponse();
    _response.setAsChunked(false);
    // _isChunkedResponse = false;
}

void Connection::printRequest(){
    if (_done){
        std::cout << "-------> Request received: <----------\n";
        _request.printRequest();
        std::cout << "-------------------------------\n";
    }
    // else
    //     std::cout << "Request not received!\n";
}

ServerConf Connection::getServer(){ return (_server);}

bool Connection::isDone(){ return (_done);}

bool Connection::isResponseDone(){ return (_response.isDone());}

time_t Connection::getTime() const { return _time; }

void Connection::setNonBlocking() {
    // int flags = fcntl(_fd, F_GETFL, 0);// F_GETFL = get file status flags
    // fcntl(_fd, F_SETFL, flags | O_NONBLOCK); // then change to non bloking
    // fcntl(_fd, F_SETFL, O_NONBLOCK);
    fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK);

}

void Connection::updateTimout(){
    _time = time(NULL);
}

std::string Connection::getConnectionHeader(Request &request) {
    std::string connection_header = request.getHeader("Connection");
    
    // Convert to lowercase for case-insensitive comparison
    std::transform(connection_header.begin(), connection_header.end(), connection_header.begin(), ::tolower);
    
    // If the request explicitly asks for close, return close
    if (connection_header == "close") {
        return "close";
    }
    
    // Default to keep-alive for HTTP/1.1
    return "keep-alive";
}

std::string Connection::checkForRedirect(Request &request, ServerConf &server) {
    std::string requested_path = request.getUri();
    std::map<std::string, LocationConf> locations = server.getLocations();
    
    // Find the best matching location (longest prefix match)
    std::string best_match = "";
    const LocationConf* best_location = NULL;
    if ((requested_path == "/profile/profile.html" ) && Request::loggedIn == false)
        return "/profile/login.html";
    if ((requested_path == "/profile" || requested_path == "/profile/"||requested_path == "/profile/login.html") && Request::loggedIn == true)
        return "/profile/profile.html";
    if (requested_path == "/logout")
        return "/profile";
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); 
         it != locations.end(); ++it) {
        if (requested_path.find(it->first) == 0 && it->first.length() > best_match.length()) {
            best_match = it->first;
            best_location = &(it->second);
        }
    }
    
    // If we found a matching location and it has a redirect URL, return it
    if (best_location != NULL) {
        std::string redirect_url = best_location->getRedirectUrl();
        if (!redirect_url.empty()) {
            MOHAMED && std::cout << "Redirect found for path: " << requested_path << " -> " << redirect_url << std::endl;
            return redirect_url;
        }
    }
    
    return ""; // No redirect found
}

void Connection::sendRedirectResponse(Request &request, const std::string &redirect_url, ServerConf &server) {
    // Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    (void)server; // Suppress unused parameter warning
    
    // Set 301 status code (permanent redirect) 
    _response.setStatus(301);
    _response.setHeader("Location", redirect_url);
    _response.setHeader("Connection", connection_header);
    _response.setHeader("Content-Type", "text/html");
    
    std::string redirect_body = "<html><body><h1>301 Moved Permanently</h1>"
                               "<p>The document has moved <a href=\"" + redirect_url + "\">here</a>.</p>"
                               "</body></html>";
    
    _response.setBody(redirect_body);
    _response.buildResponse();
    _response.setAsChunked(false);
    // _isChunkedResponse = false; // Redirects are always small responses
    
    MOHAMED && std::cout << "Sending redirect response to: " << redirect_url << std::endl;
    // return response;
}