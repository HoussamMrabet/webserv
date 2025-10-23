// #include "Connection.hpp"

// void Connection::sendDeleteResponse(Request &request, ServerConf &server) {
//     Response response_obj;
//     std::string connection_header = getConnectionHeader(request);
//     std::string requested_path = request.getUri();
//     std::string document_root;
//     std::string full_path;
    
//     // Find the matching location configuration
//     std::string location_path = "/";
//     std::map<std::string, LocationConf> locations = server.getLocations();
    
//     // Find the best matching location (longest prefix match)
//     for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); 
//          it != locations.end(); ++it) {
//         if (requested_path.find(it->first) == 0 && it->first.length() > location_path.length()) {
//             location_path = it->first;
//         }
//     }
    
//     // Check if DELETE method is allowed for this location
//     if (locations.find(location_path) != locations.end()) {
//         std::vector<std::string> allowed_methods = locations.at(location_path).getAllowedMethods();
//         bool delete_allowed = false;
//         for (std::vector<std::string>::const_iterator it = allowed_methods.begin(); 
//              it != allowed_methods.end(); ++it) {
//             if (*it == "DELETE") {
//                 delete_allowed = true;
//                 break;
//             }
//         }
//         if (!delete_allowed) {
//             response_obj.setStatus(405); // Method Not Allowed
//             response_obj.setHeader("Content-Type", "text/html");
//             response_obj.setHeader("Connection", connection_header);
//             response_obj.setBody("<html><body><h1>405 Method Not Allowed</h1>"
//                                "<p>DELETE method is not allowed for this resource.</p></body></html>");
//             _response = response_obj.buildResponse();
//             _isChunkedResponse = false;
//             return;
//         }
        
//         // Use the location's root if available, otherwise use server root
//         std::string location_root = locations.at(location_path).getRoot();
//         if (!location_root.empty()) {
//             document_root = location_root;
//         } else {
//             document_root = "./www";
//         }
//     } else {
//         document_root = "./www";
//     }
    
//     // Protected locations - prevent deletion from sensitive directories
//     std::vector<std::string> protected_paths;
//     protected_paths.push_back("/cgi-bin");
//     protected_paths.push_back("/errors");
//     protected_paths.push_back("/config");
    
//     for (std::vector<std::string>::const_iterator it = protected_paths.begin(); 
//          it != protected_paths.end(); ++it) {
//         if (requested_path.find(*it) == 0) {
//             response_obj.setStatus(403); // Forbidden
//             response_obj.setHeader("Content-Type", "text/html");
//             response_obj.setHeader("Connection", connection_header);
//             response_obj.setBody("<html><body><h1>403 Forbidden</h1>"
//                                "<p>Deletion is not allowed in protected locations.</p></body></html>");
//             _response = response_obj.buildResponse();
//             _isChunkedResponse = false;
//             return;
//         }
//     }
    
//     // Construct full path
//     if (requested_path[0] == '/') {
//         full_path = document_root + requested_path;
//     } else {
//         full_path = document_root + "/" + requested_path;
//     }
    
//     MOHAMED && std::cout << "DELETE: Full path: " << full_path << std::endl;
    
//     // Check if file exists
//     struct stat file_stat;
//     if (stat(full_path.c_str(), &file_stat) != 0) {
//         response_obj.setStatus(404);
//         response_obj.setHeader("Content-Type", "text/html");
//         response_obj.setHeader("Connection", connection_header);
//         response_obj.setBody("<html><body><h1>404 Not Found</h1>"
//                            "<p>The requested file does not exist.</p></body></html>");
//         _response = response_obj.buildResponse();
//         _isChunkedResponse = false;
//         return;
//     }
    
//     // Only allow deletion of regular files (not directories)
//     if (S_ISDIR(file_stat.st_mode)) {
//         response_obj.setStatus(403);
//         response_obj.setHeader("Content-Type", "text/html");
//         response_obj.setHeader("Connection", connection_header);
//         response_obj.setBody("<html><body><h1>403 Forbidden</h1>"
//                            "<p>Cannot delete directories.</p></body></html>");
//         _response = response_obj.buildResponse();
//         _isChunkedResponse = false;
//         return;
//     }
    
//     // Check if it's a regular file
//     if (!S_ISREG(file_stat.st_mode)) {
//         response_obj.setStatus(403);
//         response_obj.setHeader("Content-Type", "text/html");
//         response_obj.setHeader("Connection", connection_header);
//         response_obj.setBody("<html><body><h1>403 Forbidden</h1>"
//                            "<p>Can only delete regular files.</p></body></html>");
//         _response = response_obj.buildResponse();
//         _isChunkedResponse = false;
//         return;
//     }
    
//     // Security check: prevent path traversal
//     if (requested_path.find("..") != std::string::npos) {
//         response_obj.setStatus(403);
//         response_obj.setHeader("Content-Type", "text/html");
//         response_obj.setHeader("Connection", connection_header);
//         response_obj.setBody("<html><body><h1>403 Forbidden</h1>"
//                            "<p>Invalid file path.</p></body></html>");
//         _response = response_obj.buildResponse();
//         _isChunkedResponse = false;
//         return;
//     }
    
//     // Attempt to delete the file using unlink()
//     if (unlink(full_path.c_str()) == 0) {
//         MOHAMED && std::cout << "DELETE: Successfully deleted: " << full_path << std::endl;
//         response_obj.setStatus(200);
//         response_obj.setHeader("Content-Type", "text/html");
//         response_obj.setHeader("Connection", connection_header);
//         response_obj.setBody("<html><body><h1>200 OK</h1>"
//                            "<p>File successfully deleted.</p>"
//                            "<p>Path: " + requested_path + "</p></body></html>");
//         _response = response_obj.buildResponse();
//         _isChunkedResponse = false;
//     } else {
//         MOHAMED && std::cout << "DELETE: Failed to delete: " << full_path << std::endl;
        
//         // Determine error based on errno
//         int status_code = 500;
//         std::string error_msg = "Failed to delete file";
        
//         if (errno == EACCES || errno == EPERM) {
//             status_code = 403;
//             error_msg = "Permission denied";
//         } else if (errno == ENOENT) {
//             status_code = 404;
//             error_msg = "File not found";
//         }
        
//         response_obj.setStatus(status_code);
//         response_obj.setHeader("Content-Type", "text/html");
//         response_obj.setHeader("Connection", connection_header);
//         response_obj.setBody("<html><body><h1>" + to_str(status_code) + " " + 
//                            response_obj.getStatusMessage(status_code) + "</h1>"
//                            "<p>" + error_msg + ".</p></body></html>");
//         _response = response_obj.buildResponse();
//         _isChunkedResponse = false;
//     }
// }