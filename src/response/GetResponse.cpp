#include "Connection.hpp"

struct stat fileStat;

void Connection::sendGetResponse(Request &request  , ServerConf &server){
    // Create a Response object and build it step by step
    Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    if (_request.getUri() == "/profile/login.html" || _request.getUri() == "/profile" || _request.getUri() == "/profile/profile.html") {
        response_obj.setHeader("Set-Cookie", _request.getHeader("Set-Cookie"));
        if (Request::loggedIn) {
            response_obj.setHeader("X-User-Username", Request::loggedInUser.username);
            response_obj.setHeader("X-User-Email", Request::loggedInUser.email);
            response_obj.setHeader("X-User-FullName", Request::loggedInUser.fullName);
            response_obj.setHeader("X-User-Avatar", Request::loggedInUser.avatar);
            response_obj.setHeader("X-User-Job", Request::loggedInUser.job);
        }
    }
    // std::string requested_path = request.getUri();
    // std::string document_root;
    std::string full_path;
    // std::co
    
    // // Find the matching location configuration
    std::string location_path = "/";
    std::map<std::string, LocationConf> locations = server.getLocations();
    
    // // Find the best matching location (longest prefix match)
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); 
         it != locations.end(); ++it) {
        if (full_path.find(it->first) == 0 && it->first.length() > location_path.length()) {
            location_path = it->first;
        }
    }
    
    // // Always use ./www as the document root
    // document_root = this->_request.getRoot();
    //  std::string file_name = this->_request.getUri();
    // std::cout << document_root << "<------------------------->" << std::endl;
    // // Construct full path - handle the case where requested_path starts with '/'
    // if (file_name[0] == '/') {
    //     full_path = document_root + file_name;
    // } else {
    //     full_path = document_root + "/" + file_name;
    // }
    full_path = _request.getFullPath();

    // std::cout << "Requested path: " << file_name << std::endl;
    // std::cout << "Document root: " << document_root << std::endl;
    
    if (stat(full_path.c_str(), &fileStat) == 0) {
        // File exists
        if (S_ISREG(fileStat.st_mode)) {
            // Check file size to decide between regular and chunked response
            size_t file_size = static_cast<size_t>(fileStat.st_size);
            
            if (file_size >= LARGE_FILE_THRESHOLD) {
                // Use chunked transfer for large files
                _response_obj.prepareResponse(full_path);
                _isChunkedResponse = true;
                _response = ""; // Clear regular response since we're using chunked
                return;
            } else {
                // Use regular response for small files
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
                    
                    if (file_size > LARGE_FILE_THRESHOLD) {
                        // Use chunked transfer for large index files
                        _response_obj.prepareResponse(index_path);
                        _isChunkedResponse = true;
                        _response = ""; // Clear regular response since we're using chunked
                        return;
                    } else {
                        // Use regular response for small index files
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
                    // Generate directory listing
                    std::string directory_listing = generateDirectoryListing(full_path, full_path);
                    response_obj.setStatus(200);
                    response_obj.setHeader("Content-Type", "text/html");
                    response_obj.setHeader("Connection", connection_header);
                    response_obj.setBody(directory_listing);
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
        } else {
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