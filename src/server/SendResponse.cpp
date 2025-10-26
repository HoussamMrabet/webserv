#include "Connection.hpp"

bool Connection::writeResponse(){ // check if cgi or not, if cgi call cgiResponse!!!
    // First, check if we're in the middle of sending a chunked response

    CHOROUK && std::cout << "------- write fd = " << _fd << std::endl;


    if (_isChunkedResponse) {
        // std::cout << "+++ Chunked!!\n";
        std::cout << "+++ Chunked response processing\n";
        // Continue sending chunks from existing response
        if (!_response_obj.isFinished()) {
            std::string chunk = _response_obj.getResponseChunk();
            std::cout << "+++ Obtained chunk of size: " << chunk.length() << std::endl;
            if (!chunk.empty()) {
                ssize_t bytes_sent = write(_fd, chunk.c_str(), chunk.length());
                std::cout << "+++ Wrote chunk of size: " << bytes_sent << std::endl;
                std::cerr << "Chunk Content:\n" << chunk << std::endl;
                if (bytes_sent == -1) {
                    perror("Chunked write failed");
                    std::cerr << "Error details: " << strerror(errno) << std::endl;
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
    

    size_t _responseBytesSent = 0;

    if (!_isChunkedResponse) {
        // std::cout << "+++ Not chunked!!\n";

        size_t totalLen = _response.length();

        // Write remaining part of the response
        ssize_t bytes_sent = write(_fd, _response.c_str() + _responseBytesSent, totalLen - _responseBytesSent);
        // ssize_t bytes_sent = send(_fd, _response.c_str() + _responseBytesSent, totalLen - _responseBytesSent, SO_NOSIGPIPE); // no sigpipe
        updateTimout();
        if (bytes_sent == -1) {
            // perror("Write failed");
            return true;
        }

        _responseBytesSent += bytes_sent;

        if (_responseBytesSent == totalLen) {
            // Finished sending entire response
            _responseDone = true;
            _responseBytesSent = 0;  // reset for next response
            updateTimout();
        } else {
            // Partial write, wait to write remaining later
            std::cout << "Partial write: " << _responseBytesSent << "/" << totalLen << std::endl;
            // do not mark response done, wait for next call
        }
    }


    return (true);
}
