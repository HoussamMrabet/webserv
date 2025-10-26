#include "Connection.hpp"

bool Connection::writeResponse(){ // check if cgi or not, if cgi call cgiResponse!!!
    // First, check if we're in the middle of sending a chunked response

    CHOROUK && std::cout << "------- write fd = " << _fd << std::endl;


    if (_isChunkedResponse) {
        // std::cout << "+++ Chunked!!\n";
        std::cout << "+++ Chunked response processing\n";
        // Continue sending chunks from existing response
        if (!_response_obj.isFinished()) {
            if (_pendingChunk.empty()) {
                _pendingChunk = _response_obj.getResponseChunk();
                _pendingChunkOffset = 0;
                std::cout << "+++ Obtained new chunk of size: " << _pendingChunk.length() << std::endl;
            }

        if (!_pendingChunk.empty()) {
            size_t remaining = _pendingChunk.length() - _pendingChunkOffset;
        
            // Use send() with MSG_NOSIGNAL to prevent SIGPIPE
            // NO errno checking - just check return value
            ssize_t bytes_sent = send(_fd, 
                                     _pendingChunk.c_str() + _pendingChunkOffset, 
                                     remaining,
                                     MSG_NOSIGNAL);                std::cout << "+++ Sent: " << bytes_sent << " bytes\n";
            
                if (bytes_sent <= 0) {
                // Connection error or closed (we don't check errno!)
                    MOHAMED && std::cout << "Write failed or connection closed\n";
                    return (false);
                }
            
            // Track progress - handle partial writes
                _pendingChunkOffset += bytes_sent;
            
            // Check if entire chunk was sent
                if (_pendingChunkOffset >= _pendingChunk.length()) {
                // Chunk complete, clear buffer for next chunk
                    _pendingChunk.clear();
                    _pendingChunkOffset = 0;
                    MOHAMED && std::cout << "Chunk complete\n";
                }
            
                 updateTimout();
                return (true);
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
    std::string redirect_url = checkForRedirect(_request, _server);

    // CHOROUK && std::cout << C"--------- REDIRECTION FOUND!!!! -----------" << B"\n";
    if (_request.isCGI() && (_request.getStatusCode() != 200))
        close(_request.getCgiFdRead());
    if (!redirect_url.empty()) {
        _response = sendRedirectResponse(_request, redirect_url, _server);
        MOHAMED && std::cout << "Redirect Response:\n" << _response << std::endl;
        updateTimout();
    }
    else if (_request.isCGI() && (_request.getStatusCode() == 200))
    {
        if (!_cgi.readDone()){
            readCGIOutput();
            return (true);
        }
        else
            _response = setCGIHeaders();
        updateTimout();
    }
    else if ( _request.getStatusCode() != 200){
        MOHAMED && std::cout << "Error status code: " << _request.getStatusCode()  << std::endl;
        MOHAMED && std::cout << "Error message: " << _request.getMessage()  << std::endl;
        sendErrorPage(_request, _request.getStatusCode(), _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "POST"){
        // sentPostResponse(_request, _server);
        sendPostResponse(_request, _request.getStatusCode(), _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "DELETE"){
        sendDeleteResponse(_request, _server);
        MOHAMED && std::cout << _response << std::endl;
        updateTimout();
    }
    else if (_request.getStrMethod() == "GET"){ // can use pointer to member function 
        sendGetResponse(_request, _server);
        if (!_isChunkedResponse) {
            MOHAMED && std::cout << _response << std::endl;
        }
        updateTimout();
    }
    
    // Handle regular (non-chunked) response sending
    // Only executed if we're not in chunked mode or if chunked response just started
    // if (_response.empty()){
    //     _response = DEFAULT_RESPONSE;
    //     updateTimout();
    // }



    // if (!_isChunkedResponse) {
    //     std::cout << "+++ Not chunked!!\n";
    //     // std::cout << "----------> writing on fd = " << _fd << std::endl;
    //     // CHOROUK && std::cout << "writing on fd = " << _fd << std::endl;
    //     int b = write(_fd, _response.c_str(), _response.length());
    //     if (b == -1){
    //         // perror("Write failed"); // throw
    //         return (false);
    //     }
    //     CHOROUK && std::cout << "----------- STILL HERE ---------------\n";
    //     updateTimout();
    //     // Response is complete
    //     _responseDone = true;
    // }
    
    if (!_isChunkedResponse) {
        // std::cout << "+++ Not chunked!!\n";
        if (_response.empty()) {
            _responseDone = true;
            return (true);
        }
        size_t totalLen = _response.length();
        size_t remaining = totalLen - _responseBytesSent;

        // Write remaining part of the response
        if (remaining > 0) {
            // Write remaining bytes
            ssize_t bytes_sent = send(_fd, 
                                     _response.c_str() + _responseBytesSent, 
                                     remaining,
                                     MSG_NOSIGNAL);
        
            if (bytes_sent <= 0) {
                // Error or closed (no errno check!)
                MOHAMED && std::cout << "Write failed or connection closed\n";
                return (false);
            }
            _responseBytesSent += bytes_sent;
            updateTimout();
        
            MOHAMED && std::cout << "Wrote " << bytes_sent << " bytes, progress: " 
                            << _responseBytesSent << "/" << totalLen << std::endl;
        }
        if (_responseBytesSent >= totalLen) {
            MOHAMED && std::cout << "Response complete\n";
            _responseDone = true;
            _responseBytesSent = 0;  // Reset for next response
        }
    }


    return (true);
}
