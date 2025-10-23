// #include "Connection.hpp"

// void Connection::sendPostResponse(Request &request, int status_code, ServerConf &server) {
//     Response response_obj;
//     std::string connection_header = getConnectionHeader(request);
//     (void)server; // Suppress unused parameter warning
    
//     // Since POST logic is already handled in Request class, just send success response
//     response_obj.setStatus(status_code);
//     response_obj.setHeader("Content-Type", "text/html");
//     response_obj.setHeader("Connection", connection_header);
    
//     std::string success_body = "<html><body><h1>POST Request Successful</h1>"
//                               "<p>Your POST request has been processed successfully.</p>"
//                               "</body></html>";
    
//     response_obj.setBody(success_body);
//     _response = response_obj.buildResponse();
//     _isChunkedResponse = false;
// }

// std::string Connection::getConnectionHeader(Request &request) {
//     std::string connection_header = request.getHeader("Connection");
    
//     // Convert to lowercase for case-insensitive comparison
//     std::transform(connection_header.begin(), connection_header.end(), connection_header.begin(), ::tolower);
    
//     // If the request explicitly asks for close, return close
//     if (connection_header == "close") {
//         return "close";
//     }
    
//     // Default to keep-alive for HTTP/1.1
//     return "keep-alive";
// }

