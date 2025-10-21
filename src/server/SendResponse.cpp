#include "Connection.hpp"

bool Connection::writeResponse() {
    CHOROUK && std::cout << "------- write fd = " << _fd << std::endl;
    
    // If we're in chunked mode, continue sending chunks
    if (_isChunkedResponse) {
        CHOROUK && std::cout << "Sending chunked response" << std::endl;
        
        if (!_response_obj.isFinished()) {
            std::string chunk = _response_obj.getResponseChunk();
            if (!chunk.empty()) {
                ssize_t bytes_sent = write(_fd, chunk.c_str(), chunk.length());
                if (bytes_sent == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        return true; // Would block, try again later
                    }
                    perror("Chunked write failed");
                    return false;
                }
                if (bytes_sent < (ssize_t)chunk.length()) {
                    MOHAMED && std::cout << "Partial write: " << bytes_sent << "/" << chunk.length() << std::endl;
                    // TODO: Handle partial writes by storing remaining data
                }
                MOHAMED && std::cout << "Sent chunk of size: " << bytes_sent << std::endl;
                updateTimout();
                return true;
            }
        }
        
        // All chunks sent
        if (_response_obj.isFinished()) {
            MOHAMED && std::cout << "Chunked response complete" << std::endl;
            _responseDone = true;
            return true;
        }
    }
    
    // If response hasn't been built yet, build it now
    if (_response.empty()) {
        // Check for redirects first
        std::string redirect_url = checkForRedirect(_request, _server);
        
        if (_request.isCGI() && (_request.getStatusCode() != 200)) {
            close(_request.getCgiFdRead());
        }
        
        if (!redirect_url.empty()) {
            _response = sendRedirectResponse(_request, redirect_url, _server);
            MOHAMED && std::cout << "Redirect Response prepared" << std::endl;
        }
        else if (_request.isCGI() && (_request.getStatusCode() == 200)) {
            if (!_cgi.readDone()) {
                // Still waiting for CGI output
                CHOROUK && std::cout << "Waiting for CGI output" << std::endl;
                return true;
            }
            _response = setCGIHeaders();
            MOHAMED && std::cout << "CGI Response prepared" << std::endl;
        }
        else if (_request.getStatusCode() != 200) {
            sendErrorPage(_request, _request.getStatusCode(), _server);
            MOHAMED && std::cout << "Error response prepared: " << _request.getStatusCode() << std::endl;
        }
        else if (_request.getStrMethod() == "POST") {
            sendPostResponse(_request, _request.getStatusCode(), _server);
            MOHAMED && std::cout << "POST response prepared" << std::endl;
        }
        else if (_request.getStrMethod() == "DELETE") {
            sendDeleteResponse(_request, _server);
            MOHAMED && std::cout << "DELETE response prepared" << std::endl;
        }
        else if (_request.getStrMethod() == "GET") {
            sendGetResponse(_request, _server);
            if (_isChunkedResponse) {
                MOHAMED && std::cout << "GET chunked response started" << std::endl;
                return true; // Will continue in chunked mode on next call
            }
            MOHAMED && std::cout << "GET response prepared" << std::endl;
        }
        
        updateTimout();
    }
    
    // Send regular (non-chunked) response
    if (!_isChunkedResponse && !_response.empty()) {
        CHOROUK && std::cout << "Sending regular response, size: " << _response.length() << std::endl;
        
        ssize_t bytes_sent = write(_fd, _response.c_str(), _response.length());
        if (bytes_sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return true; // Would block, try again later
            }
            perror("Write failed");
            return false;
        }
        
        if (bytes_sent < (ssize_t)_response.length()) {
            MOHAMED && std::cout << "Partial write: " << bytes_sent << "/" << _response.length() << std::endl;
            // Remove sent portion from buffer
            _response = _response.substr(bytes_sent);
            updateTimout();
            return true; // Continue on next POLLOUT
        }
        
        CHOROUK && std::cout << "Regular response sent completely" << std::endl;
        updateTimout();
        _responseDone = true;
    }
    
    return true;
}