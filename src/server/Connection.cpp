#include "Connection.hpp"
#include <algorithm>
ServerConf Connection::_server;
// ServerConf Connection::_server = ConfigBuilder::generateServers("config/config.conf").back();
struct stat fileStat;

#define DEFAULT_RESPONSE "HTTP/1.1 200 OK\r\n" \
                         "Content-Length: 10\r\n" \
                         "Content-Type: text/plain\r\n" \
                         "Connection: keep-alive\r\n" \
                         "\r\n" \
                         "Received!!\r\n" // Connection keep-alive but still ends !!!!!

Connection::~Connection(){ 
    CHOROUK && std::cout << "***********  connection destructor called!!! ***********\n";
    delete _request;
}

Connection::Connection(int fd, ServerConf& server): _time(time(NULL)),
                                                    _done(false),
                                                    _responseDone(false),
                                                    _isChunkedResponse(false)/*....*/ {
    _fd = accept(fd, NULL, NULL);
    _server = server;
    _request = new Request();
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
    std::cout << "[Sun Sep 14 15:23:11 2025] [::1]:52214 [302]: ";
}

Connection::Connection(const Connection& connection){
    _fd = connection._fd;
    _time = connection._time;
    _buffer = connection._buffer;
    // _request = connection._request;
    _done = connection._done;
    _responseDone = connection._responseDone;
    _isChunkedResponse = connection._isChunkedResponse;
}

int Connection::getFd() const{ return (_fd);}

bool Connection::readRequest(){
    char buffer[1024] = {0};
    ssize_t bytesRead = 0;

    if  (!_request->isDone() && this->_responseDone == false)
    {
        CHOROUK && std::cout << "------- read fd = " << _fd << std::endl;
        bytesRead = read(_fd, buffer, sizeof(buffer));
        CHOROUK && std::cout << "------- bytes = " << bytesRead << std::endl;
        if (bytesRead > 0)
        {
            _buffer.append(buffer, bytesRead);
            _request->parseRequest(_buffer);
            _buffer.clear();
            updateTimout();  // update activity timestamp
            // else continue;
        }
        else
        {
            _request->parseRequest();
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
    _done = _request->isDone();
    // CHOROUK && _request->printRequest();


    // if (_request->isDone()) // remove from here and add to webserv in order to add pipe_fds to pollfds
    //     _response = CGI::executeCGI(*_request);
    return (true);
}

bool Connection::writeResponse(){ // check if cgi or not, if cgi call cgiResponse!!!
    // First, check if we're in the middle of sending a chunked response

    CHOROUK && std::cout << "------- write fd = " << _fd << std::endl;
    std::cout << M"" << _request->getUri() << B"\n";
   
    if (_isChunkedResponse) {
        // Continue sending chunks from existing response
        if (!_response_obj.isFinished()) {
            std::string chunk = _response_obj.getResponseChunk();
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
            _responseDone = true;
            _isChunkedResponse = false;
            return (true);
        }
    }
    
    // Regular response processing - only if not in chunked mode
    // Check for redirects first
    std::string redirect_url = checkForRedirect(*_request, _server);
    // CHOROUK && std::cout << C"--------- REDIRECTION FOUND!!!! -----------" << B"\n";
    if (!redirect_url.empty()) {
        _response = sendRedirectResponse(*_request, redirect_url, _server);
        MOHAMED && std::cout << "Redirect Response:\n" << _response << std::endl;
        updateTimout();
    }
    else if (_request->isCGI())
    {
        CHOROUK && std:: cout << M"IT IS CGI!!!!!\n";
        _response = CGI::executeCGI(*_request, _server);
        // _cgiFd = CGI::getFd();
        updateTimout();
    }
    else if ( _request->getStatusCode() != 200){
        MOHAMED && std::cout << "Error status code: " << _request->getStatusCode()  << std::endl;
        MOHAMED && std::cout << "Error message: " << _request->getMessage()  << std::endl;
        sendErrorPage(*_request, _request->getStatusCode(), _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request->getStrMethod() == "POST"){
        // sentPostResponse(*_request, _server);
        sendPostResponse(*_request, _request->getStatusCode(), _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request->getStrMethod() == "DELETE"){
        // sendDeleteResponse(*_request, _server);
        sendErrorPage(*_request, 501, _server); // not implemented
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request->getStrMethod() == "GET"){ // can use pointer to member function 
        sendGetResponse(*_request, _server);
        if (!_isChunkedResponse) {
            MOHAMED && std::cout << _response << std::endl;
        }
        updateTimout();
    }
    
    // Handle regular (non-chunked) response sending
    // Only executed if we're not in chunked mode or if chunked response just started
    if (_response.empty()){
        _response = DEFAULT_RESPONSE;
        updateTimout();
    }
    if (!_isChunkedResponse) {
        CHOROUK && std::cout << "writing on fd = " << _fd << std::endl;
        int b = write(_fd, _response.c_str(), _response.length());
        if (b == -1){
            // perror("Write failed"); // throw
            return (false);
        }
        CHOROUK && std::cout << "----------- STILL HERE ---------------\n";
        updateTimout();
        // Response is complete
        _responseDone = true;
    }
    
    return (true);
}

// std::string Connection::getRequestMethod(){
//     t_method method = _request->getMethod();
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

void Connection::sendErrorPage(Request &request, int code, ServerConf &server){
    Response response_obj;
    std::string error_page;
    std::map<std::string, std::string> error_pages = server.getErrorPages();
    std::string connection_header = getConnectionHeader(request);
    
    // Check if there's a custom error page defined in config
    if (error_pages.find(to_str(code)) != error_pages.end()) {
        error_page = error_pages[to_str(code)];
        // Attach ./www/errors/ to the error page path
        std::string full_error_path = "./www/errors/" + error_page;
        if (response_obj.fileExists(full_error_path)) {
            response_obj.setStatus(code);
            response_obj.setBodyFromFile(full_error_path);
            response_obj.setHeader("Content-Type", response_obj.getContentType(full_error_path));
            response_obj.setHeader("Connection", connection_header);
            _response = response_obj.buildResponse();
            _isChunkedResponse = false;
            return;
        }
    }
    
    // Fallback: check for standard error pages in ./www/errors/
    std::string standard_error_page = "./www/errors/" + to_str(code) + ".html";
    if (response_obj.fileExists(standard_error_page)) {
        response_obj.setStatus(code);
        response_obj.setBodyFromFile(standard_error_page);
        response_obj.setHeader("Content-Type", response_obj.getContentType(standard_error_page));
        response_obj.setHeader("Connection", connection_header);
        _response = response_obj.buildResponse();
        _isChunkedResponse = false;
        return;
    }
    
    // Default error message if no custom page is found or file doesn't exist
    response_obj.setStatus(code);
    response_obj.setBody("<html><body><h1>" + to_str(code) + " " + response_obj.getStatusMessage(code) + "</h1></body></html>");
    response_obj.setHeader("Content-Type", "text/html");
    response_obj.setHeader("Connection", connection_header);
    _response = response_obj.buildResponse();
    _isChunkedResponse = false;
}

void Connection::sendGetResponse(Request &request  , ServerConf &server){
    // Create a Response object and build it step by step
    Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    if (_request->getUri() == "/profile/login.html" || _request->getUri() == "/profile" || _request->getUri() == "/profile/profile.html") {
        response_obj.setHeader("Set-Cookie", _request->getHeader("Set-Cookie"));
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
    }
    
    // Always use ./www as the document root
    document_root = "./www";
    
    // Construct full path - handle the case where requested_path starts with '/'
    if (requested_path[0] == '/') {
        full_path = document_root + requested_path;
    } else {
        full_path = document_root + "/" + requested_path;
    }
    
    MOHAMED && std::cout << "Requested path: " << requested_path << std::endl;
    MOHAMED && std::cout << "Document root: " << document_root << std::endl;
    MOHAMED && std::cout << "Full path: " << full_path << std::endl;
    
    if (stat(full_path.c_str(), &fileStat) == 0) {
        // File exists
        if (S_ISREG(fileStat.st_mode)) {
            // Check file size to decide between regular and chunked response
            size_t file_size = static_cast<size_t>(fileStat.st_size);
            MOHAMED && std::cout << "File size: " << file_size << " bytes" << std::endl;
            
            if (file_size > LARGE_FILE_THRESHOLD) {
                // Use chunked transfer for large files
                MOHAMED && std::cout << "Using chunked transfer for large file" << std::endl;
                _response_obj.prepareResponse(full_path);
                _isChunkedResponse = true;
                _response = ""; // Clear regular response since we're using chunked
                return;
            } else {
                // Use regular response for small files
                MOHAMED && std::cout << "Using regular response for small file" << std::endl;
                response_obj.setStatus(200);
                response_obj.setBodyFromFile(full_path);
                response_obj.setHeader("Connection", connection_header);
                _response = response_obj.buildResponse();
                _isChunkedResponse = false;
                return;
            }
        } else if (S_ISDIR(fileStat.st_mode)) {
            bool auto_index = false;
            std::vector<std::string> index_files;
            
            if (locations.find(location_path) != locations.end()) {
                auto_index = locations.at(location_path).getAutoIndex();
                index_files = locations.at(location_path).getIndex();
            }
            
            if (index_files.empty()) {
                index_files = server.getIndex();
            }
            
            // Try to serve index files
            bool index_found = false;
            for (std::vector<std::string>::const_iterator it = index_files.begin(); 
                 it != index_files.end(); ++it) {
                std::string index_path = full_path;
                if (index_path[index_path.length() - 1] != '/') {
                    index_path += "/";
                }
                index_path += *it;
                
                if (stat(index_path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                    // Check file size to decide between regular and chunked response
                    size_t file_size = static_cast<size_t>(fileStat.st_size);
                    MOHAMED && std::cout << "Index file size: " << file_size << " bytes" << std::endl;
                    
                    if (file_size > LARGE_FILE_THRESHOLD) {
                        // Use chunked transfer for large index files
                        MOHAMED && std::cout << "Using chunked transfer for large index file" << std::endl;
                        _response_obj.prepareResponse(index_path);
                        _isChunkedResponse = true;
                        _response = ""; // Clear regular response since we're using chunked
                        return;
                    } else {
                        // Use regular response for small index files
                        MOHAMED && std::cout << "Using regular response for small index file" << std::endl;
                        response_obj.setStatus(200);
                        response_obj.setBodyFromFile(index_path);
                        response_obj.setHeader("Connection", connection_header);
                        _response = response_obj.buildResponse();
                        _isChunkedResponse = false;
                        return;
                    }
                }
            }
            
            if (!index_found) {
                if (auto_index) {
                    response_obj.setStatus(200);
                    response_obj.setHeader("Content-Type", "text/html");
                    response_obj.setHeader("Connection", connection_header);
                    response_obj.setBody("<html><body><h1>Directory listing not implemented yet</h1></body></html>");
                    _response = response_obj.buildResponse();
                    _isChunkedResponse = false;
                    return;
                } else {
                    // Directory listing forbidden
                    response_obj.setStatus(403);
                    response_obj.setBody("Forbidden - Directory listing disabled");
                    response_obj.setHeader("Content-Type", "text/html");
                    response_obj.setHeader("Connection", connection_header);
                    _response = response_obj.buildResponse();
                    _isChunkedResponse = false;
                    return;
                }
            }
            MOHAMED && std::cout << "It's a directory." << std::endl;
        } else {
            MOHAMED && std::cout << "It's neither a regular file nor a directory." << std::endl;
            response_obj.setStatus(403);
            response_obj.setBody("Forbidden");
            response_obj.setHeader("Content-Type", "text/html");
            response_obj.setHeader("Connection", connection_header);
            _response = response_obj.buildResponse();
            _isChunkedResponse = false;
            return;
        }
    } else {
        // File does not exist
        MOHAMED && std::cout << "File does not exist: " << full_path << std::endl;
        response_obj.setStatus(404);
        response_obj.setBody("File not found");
        response_obj.setHeader("Content-Type", "text/html");
        response_obj.setHeader("Connection", connection_header);
        _response = response_obj.buildResponse();
        _isChunkedResponse = false;
        return;
    }   
    
    // This should not be reached now since we handle all cases above
    _response = response_obj.buildResponse();
    _isChunkedResponse = false;
}

void Connection::sendPostResponse(Request &request, int status_code, ServerConf &server) {
    Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    (void)server; // Suppress unused parameter warning
    
    // Since POST logic is already handled in Request class, just send success response
    response_obj.setStatus(status_code);
    response_obj.setHeader("Content-Type", "text/html");
    response_obj.setHeader("Connection", connection_header);
    
    std::string success_body = "<html><body><h1>POST Request Successful</h1>"
                              "<p>Your POST request has been processed successfully.</p>"
                              "</body></html>";
    
    response_obj.setBody(success_body);
    _response = response_obj.buildResponse();
    _isChunkedResponse = false;
}

void Connection::printRequest(){
    if (_done){
        std::cout << "-------> Request received: <----------\n";
        _request->printRequest();
        std::cout << "-------------------------------\n";
    }
    // else
    //     std::cout << "Request not received!\n";
}

ServerConf Connection::getServer(){ return (_server);}

bool Connection::isDone(){ return (_done);}

bool Connection::isResponseDone(){ return (_responseDone);}

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

std::string Connection::sendRedirectResponse(Request &request, const std::string &redirect_url, ServerConf &server) {
    Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    (void)server; // Suppress unused parameter warning
    
    // Set 301 status code (permanent redirect) 
    response_obj.setStatus(301);
    response_obj.setHeader("Location", redirect_url);
    response_obj.setHeader("Connection", connection_header);
    response_obj.setHeader("Content-Type", "text/html");
    
    std::string redirect_body = "<html><body><h1>301 Moved Permanently</h1>"
                               "<p>The document has moved <a href=\"" + redirect_url + "\">here</a>.</p>"
                               "</body></html>";
    
    response_obj.setBody(redirect_body);
    std::string response = response_obj.buildResponse();
    _isChunkedResponse = false; // Redirects are always small responses
    
    MOHAMED && std::cout << "Sending redirect response to: " << redirect_url << std::endl;
    return response;
}