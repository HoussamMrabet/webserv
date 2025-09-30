#include "Connection.hpp"

struct stat fileStat;

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

std::string to_str(int n){
    std::stringstream ss; ss << n;
    return (ss.str());
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
                    CGI cgi;
                    std::string res = cgi.executeCGI(_request, _server, full_path);
                    _cgiFd = cgi.getFd();
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