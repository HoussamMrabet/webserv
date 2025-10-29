#include "Connection.hpp"

bool Connection::writeResponse(){

    if (_isChunkedResponse) {
        if (!_response_obj.isFinished()) {
            if (_currentChunk.empty()) {
                _currentChunk = _response_obj.getResponseChunk();
                if (_currentChunk.empty()) {
                    return true;
                }
            }
            
            ssize_t bytes_sent = send(_fd, _currentChunk.c_str(), _currentChunk.length(), SO_NOSIGPIPE);
            if (bytes_sent == -1) {
                return false;
            } else if (bytes_sent == 0) {
                return false;
            } else if (bytes_sent < (ssize_t)_currentChunk.length()) {
                _currentChunk = _currentChunk.substr(bytes_sent);
                return true;
            }
            
            _currentChunk.clear();
            return true;
        } else {
            _responseDone = true;
            _isChunkedResponse = false;
            _currentChunk.clear();
            return true;
        }
    }
    std::string redirect_url = checkForRedirect(_request, _server);

    if (_request.isCGI() && (_request.getStatusCode() != 200))
        close(_request.getCgiFdRead());
    if (!redirect_url.empty()) {
        _response = sendRedirectResponse(_request, redirect_url, _server);
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
        sendErrorPage(_request, _request.getStatusCode(), _server);
        updateTimout();
    }
    else if (_request.getStrMethod() == "POST"){
        sendPostResponse(_request, _request.getStatusCode(), _server);
        updateTimout();
    }
    else if (_request.getStrMethod() == "DELETE"){
        sendDeleteResponse(_request, _server);
        updateTimout();
    }
    else if (_request.getStrMethod() == "GET"){
        sendGetResponse(_request, _server);
        updateTimout();
    }    

    size_t _responseBytesSent = 0;

    if (!_isChunkedResponse) {

        size_t totalLen = _response.length();

        ssize_t bytes_sent = write(_fd, _response.c_str() + _responseBytesSent, totalLen - _responseBytesSent);
        updateTimout();
        if (bytes_sent == -1) {
            return true;
        }

        _responseBytesSent += bytes_sent;

        if (_responseBytesSent == totalLen) {
            _responseDone = true;
            _responseBytesSent = 0;
            updateTimout();
        }
    }


    return (true);
}
