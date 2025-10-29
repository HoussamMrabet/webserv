#include "Connection.hpp"

void Connection::sendErrorPage(Request &request, int code, ServerConf &server){
    Response response_obj;
    std::string error_page;
    std::map<std::string, std::string> error_pages = server.getErrorPages();
    std::string connection_header = getConnectionHeader(request);
    
    // Check if there's a custom error page defined in config
    if (error_pages.find(to_str(code)) != error_pages.end()) {
        error_page = error_pages[to_str(code)];
        // Attach ./www/errors/ to the error page path
        std::string full_error_path = error_page;
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
    std::string standard_error_page = request.getRoot() + "/errors/" + to_str(code) + ".html";
    if (response_obj.fileExists(standard_error_page)) {
        response_obj.setStatus(code);
        response_obj.setBodyFromFile(standard_error_page);
        response_obj.setHeader("Content-Type", response_obj.getContentType(standard_error_page));
        response_obj.setHeader("Connection", connection_header);
        _response = response_obj.buildResponse();
        _isChunkedResponse = false;
        return;
    }
    response_obj.setStatus(code);
    response_obj.setBody("<html><body><h1>" + to_str(code) + " " + response_obj.getStatusMessage(code) + "</h1></body></html>");
    response_obj.setHeader("Content-Type", "text/html");
    response_obj.setHeader("Connection", connection_header);
    _response = response_obj.buildResponse();
    _isChunkedResponse = false;
}