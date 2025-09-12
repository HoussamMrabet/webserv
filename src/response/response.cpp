/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 15:34:45 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/08/22 16:13:16 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"


std::map<int, std::string> Response::status_map;


Response::Response() : http_version("HTTP/1.1"), content_length(0), 
                      is_chunked(false), connection_close(true), 
                      is_ready(false), has_body(false), bytes_sent(0), headers_sent(false) {
    init_status_map();
}

Response::Response(int status_code) : http_version("HTTP/1.1"), content_length(0), 
                      is_chunked(false), connection_close(true), 
                      is_ready(false), has_body(false), bytes_sent(0), headers_sent(false) {
    init_status_map();
    setStatus(status_code);
}

Response::~Response() {
    if (file_stream.is_open()) {
        file_stream.close();
    }
}

void Response::init_status_map() {
    if (!status_map.empty()) return;
    status_map[200] = "OK";
    status_map[201] = "Created";
    status_map[204] = "No Content";
    status_map[301] = "Moved Permanently";
    status_map[302] = "Found";
    status_map[304] = "Not Modified";
    status_map[400] = "Bad Request";
    status_map[403] = "Forbidden";
    status_map[404] = "Not Found";
    status_map[405] = "Method Not Allowed";
    status_map[413] = "Payload Too Large";
    status_map[415] = "Unsupported Media Type";
    status_map[500] = "Internal Server Error";
    status_map[501] = "Not Implemented";
    status_map[505] = "HTTP Version Not Supported";
}


void Response::setStatus(int status_code) {
    std::ostringstream oss;
    oss << status_code;
    this->status = oss.str();
    this->status_message = getStatusMessage(status_code);
}

void Response::setHeader(const std::string& key, const std::string& value) {
    this->headers[key] = value;
}

void Response::setHttpVersion(const std::string& version) {
    this->http_version = version;
    
    // Connection behavior can be flexible - server decides when to close
    if (version == "HTTP/1.0") {
        this->connection_close = true;  
    } else if (version == "HTTP/1.1") {
        this->connection_close = false; 
    }
}

void Response::setBody(const std::string& content) {
    this->body = content;
    this->content_length = content.length();
    this->has_body = !content.empty();
    
    std::ostringstream oss;
    oss << content_length;
    setHeader("Content-Length", oss.str());
}

void Response::setBodyFromFile(const std::string& file_path) {
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        setStatus(404);
        setBody("File not found");
        return;
    }
    
    std::ostringstream buffer;
    buffer << file.rdbuf();
    setBody(buffer.str());
    
    setHeader("Content-Type", getContentType(file_path));
}

std::string Response::buildResponse() {
    std::ostringstream response;
    
    response << http_version << " " << status << " " << status_message << CRLF;
    
    for (std::map<std::string, std::string>::iterator it = headers.begin(); 
         it != headers.end(); ++it) {
        response << it->first << ": " << it->second << CRLF;
    }
    
    response << CRLF;
    
    if (has_body) {
        response << body;
    }
    
    return response.str();
}


std::string Response::buildGetResponse(const std::string& file_path) {
    if (!fileExists(file_path)) {
        setStatus(404);
        setBody("File not found");
        setHeader("Content-Type", "text/html");
        setHeader("Connection", "close"); // HTTP/1.0 default
        return buildResponse();
    }
    
    setStatus(200);
    setBodyFromFile(file_path);
    setHeader("Connection", "close"); // HTTP/1.0 default connection behavior
    return buildResponse();
}


std::string Response::buildPostResponse() {
    setStatus(201);
    setBody("Resource created successfully");
    setHeader("Content-Type", "text/plain");
    setHeader("Connection", "close"); // HTTP/1.0 default
    return buildResponse();
}

// Build DELETE response
std::string Response::buildDeleteResponse(const std::string& file_path) {
    if (!fileExists(file_path)) {
        setStatus(404);
        setBody("File not found");
        setHeader("Content-Type", "text/plain");
        setHeader("Connection", "close"); // HTTP/1.0 default
        return buildResponse();
    }
    
  
    if (std::remove(file_path.c_str()) == 0) {
        setStatus(204); // No Content - successful deletion
        setHeader("Connection", "close"); // HTTP/1.0 default
    } else {
        setStatus(500);
        setBody("Failed to delete file");
        setHeader("Content-Type", "text/plain");
        setHeader("Connection", "close"); // HTTP/1.0 default
    }
    
    return buildResponse();
}


std::string Response::getContentType(const std::string& file_path) {
    MimeTypes mime;
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string extension = file_path.substr(dot_pos);
        std::string mime_type = mime.getMimeType(extension);
        if (mime_type != "NONE") {
            return mime_type;
        }
    }
    return "application/octet-stream"; 
}


bool Response::fileExists(const std::string& file_path) {
    std::ifstream file(file_path.c_str());
    return file.good();
}


std::string Response::getStatusMessage(int code) {
    std::map<int, std::string>::iterator it = status_map.find(code);
    if (it != status_map.end()) {
        return it->second;
    }
    return "Unknown Status";
}

std::string Response::getResponse(int code){
    std::string response;
    switch (code)
    {
        case 200:
            response = "HTTP/1.0 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nDone!";
            break;
        case 400:
            response = "HTTP/1.0 400 Bad Request\r\nContent-Length: 11\r\nConnection: close\r\n\r\nBad Request";
            break;
        case 404:
            response = "HTTP/1.0 404 Not Found\r\nContent-Length: 9\r\nConnection: close\r\n\r\nNot Found";
            break;
        case 413:
            response = "HTTP/1.0 413 Request Entity Too Large\r\nContent-Length: 24\r\nConnection: close\r\n\r\nRequest Entity Too Large";
            break;
        case 415:
            response = "HTTP/1.0 415 Unsupported Media Type\r\nContent-Length: 26\r\nConnection: close\r\n\r\nUnsupported Media Type";
            break;
        case 403:
            response = "HTTP/1.0 403 Forbidden\r\nContent-Length: 9\r\nConnection: close\r\n\r\nForbidden";
            break;
        case 405:
            response = "HTTP/1.0 405 Method Not Allowed\r\nContent-Length: 18\r\nConnection: close\r\n\r\nMethod Not Allowed";
            break;
        case 500:
            response = "HTTP/1.0 500 Internal Server Error\r\nContent-Length: 21\r\nConnection: close\r\n\r\nInternal Server Error";
            break;
        case 501:
            response = "HTTP/1.0 501 Not Implemented\r\nContent-Length: 17\r\nConnection: close\r\n\r\nNot Implemented";
            break;
        case 505:
            response = "HTTP/1.0 505 HTTP Version Not Supported\r\nContent-Length: 29\r\nConnection: close\r\n\r\nHTTP Version Not Supported";
            break;
        default:
            response = "HTTP/1.0 500 Internal Server Error\r\nContent-Length: 21\r\nConnection: close\r\n\r\nInternal Server Error";
            break;
    }
    return (response);
}

// ============== CHUNKED RESPONSE METHODS ==============

// Prepare file response for chunked sending
void Response::prepareFileResponse(const std::string& file_path) {
    this->file_path = file_path;
    this->bytes_sent = 0;
    this->headers_sent = false;
    this->is_ready = false;
    
    if (!fileExists(file_path)) {
        setStatus(404);
        setBody("File not found");
        setHeader("Content-Type", "text/html");
        setHeader("Connection", "close"); 
        this->is_ready = true; 
        return;
    }
    

    if (file_stream.is_open()) {
        file_stream.close();
    }
    
    file_stream.open(file_path.c_str(), std::ios::binary);
    if (!file_stream.is_open()) {
        setStatus(500);
        setBody("Cannot open file");
        setHeader("Content-Type", "text/html");
        setHeader("Connection", "close"); 
        this->is_ready = true; 
        return;
    }

    setStatus(200);
    this->content_length = getFileSize(file_path);
    setHeader("Content-Type", getContentType(file_path));
    setHeader("Connection", "close");
    

    std::ostringstream response;
    response << http_version << " " << status << " " << status_message << CRLF;
    
   
    std::ostringstream content_len;
    content_len << content_length;
    setHeader("Content-Length", content_len.str());
    
    for (std::map<std::string, std::string>::iterator it = headers.begin(); 
         it != headers.end(); ++it) {
        response << it->first << ": " << it->second << CRLF;
    }
    response << CRLF;
    
    this->cached_headers = response.str();
}


std::string Response::getNextChunk(size_t chunk_size) {
    std::string chunk;
    
    // First call: send headers
    if (!headers_sent) {
        headers_sent = true;
        
        // If it's an error response or small file, return complete response
        if (!body.empty()) {
            chunk = cached_headers + body;
            is_ready = true;
            return chunk;
        }
        
        // For file streaming, return just headers first
        chunk = cached_headers;
        return chunk;
    }
    
    // Subsequent calls: send file content
    if (!file_stream.is_open() || file_stream.eof()) {
        is_ready = true;
        return "";
    }
    
    // Read next chunk from file
    char buffer[BUFFER_SIZE];
    size_t actual_chunk_size = (chunk_size > BUFFER_SIZE) ? BUFFER_SIZE : chunk_size;
    file_stream.read(buffer, actual_chunk_size);
    std::streamsize bytes_read = file_stream.gcount();
    
    if (bytes_read > 0) {
        chunk.assign(buffer, bytes_read);
        bytes_sent += bytes_read;
    }
    
    // Check if we've sent everything
    if (file_stream.eof() || bytes_sent >= content_length) {
        is_ready = true;
        file_stream.close();
    }
    
    return chunk;
}

// Check if response is complete
bool Response::isResponseComplete() const {
    return is_ready;
}

// Reset for reuse
void Response::reset() {
    if (file_stream.is_open()) {
        file_stream.close();
    }
    
    status.clear();
    status_message.clear();
    headers.clear();
    body.clear();
    file_path.clear();
    cached_headers.clear();
    
    content_length = 0;
    bytes_sent = 0;
    is_chunked = false;
    connection_close = false;
    is_ready = false;
    has_body = false;
    headers_sent = false;
}


size_t Response::getFileSize(const std::string& file_path) {
    std::ifstream file(file_path.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    size_t size = file.tellg();
    file.close();
    return size;
}


void Response::prepareResponse(const std::string& file_path) {
    this->file_path = file_path;
    this->bytes_sent = 0;
    this->headers_sent = false;
    this->is_ready = false;
    
    if (!fileExists(file_path)) {
        setStatus(404);
        setBody("File not found");
        setHeader("Content-Type", "text/html");
        return;
    }
    
    // File exists - set up for streaming
    if (file_stream.is_open()) {
        file_stream.close();
    }
    
    file_stream.open(file_path.c_str(), std::ios::binary);
    if (!file_stream.is_open()) {
        // Cannot open file - prepare error response
        setStatus(500);
        setBody("Cannot open file");
        setHeader("Content-Type", "text/html");
        return;
    }
    
    // Success - prepare file response
    setStatus(200);
    this->content_length = getFileSize(file_path);
    setHeader("Content-Type", getContentType(file_path));
    
    // Build headers for any HTTP version
    std::ostringstream response;
    response << http_version << " " << status << " " << status_message << CRLF;
    
    // Set Content-Length header
    std::ostringstream content_len;
    content_len << content_length;
    setHeader("Content-Length", content_len.str());
    
    // Add connection header based on HTTP version
    if (http_version == "HTTP/1.0") {
        setHeader("Connection", "close");
    } else {
        setHeader("Connection", "keep-alive");
    }
    
    for (std::map<std::string, std::string>::iterator it = headers.begin(); 
         it != headers.end(); ++it) {
        response << it->first << ": " << it->second << CRLF;
    }
    response << CRLF;
    
    this->cached_headers = response.str();
}

// Get next chunk of response - call repeatedly until isFinished() returns true
std::string Response::getResponseChunk(size_t chunk_size) {
    std::string chunk;
    
    // First call: send headers
    if (!headers_sent) {
        headers_sent = true;
        
        // If it's an error response (has body but no file stream), return complete response
        if (!body.empty() && !file_stream.is_open()) {
            chunk = cached_headers + body;
            is_ready = true; // Response is complete
            return chunk;
        }
        
        // For file streaming, return just headers first
        chunk = cached_headers;
        return chunk;
    }
    
    // Subsequent calls: send file content
    if (!file_stream.is_open() || file_stream.eof()) {
        is_ready = true; // Response is complete
        return "";
    }
    
    // Read next chunk from file
    char buffer[BUFFER_SIZE];
    size_t actual_chunk_size = (chunk_size > BUFFER_SIZE) ? BUFFER_SIZE : chunk_size;
    file_stream.read(buffer, actual_chunk_size);
    std::streamsize bytes_read = file_stream.gcount();
    
    if (bytes_read > 0) {
        chunk.assign(buffer, bytes_read);
        bytes_sent += bytes_read;
    }
    
    // Check if we've sent everything
    if (file_stream.eof() || bytes_sent >= content_length) {
        is_ready = true; // Response is complete - server can close connection
        file_stream.close();
    }
    
    return chunk;
}

// Check if response is finished - server uses this to know when to close connection
bool Response::isFinished() const {
    return is_ready;
}