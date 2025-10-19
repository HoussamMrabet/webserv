#include "Connection.hpp"

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



