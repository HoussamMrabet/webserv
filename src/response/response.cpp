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
    std::cout << C"Response default constructor called" << B"\n";
    init_status_map();
}

Response::Response(int status_code) : http_version("HTTP/1.1"), content_length(0), 
                      is_chunked(false), connection_close(true), 
                      is_ready(false), has_body(false), bytes_sent(0), headers_sent(false) {
    std::cout << C"Response parameterized constructor called" << B"\n";
    init_status_map();
    setStatus(status_code);
}

Response::~Response() {
    std::cout << C"Response destructor called" << B"\n";
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

void Response::setAsChunked(bool chunked) {
    this->is_chunked = chunked;
    // if (chunked) {
    //     setHeader("Transfer-Encoding", "chunked");
    //     headers.erase("Content-Length"); // Remove Content-Length if chunked
    // } else {
    //     headers.erase("Transfer-Encoding");
    // }
}

void Response::setAsDone() {
    this->is_done = true;
}

bool Response::isDone() const {
    return this->is_done;
}

bool Response::isChunked() const {
    return this->is_chunked;
}

void Response::buildResponse() {
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
    
    response_str = response.str();
}


void Response::buildGetResponse(const std::string& file_path) {
    if (!fileExists(file_path)) {
        setStatus(404);
        setBody("File not found");
        setHeader("Content-Type", "text/html");
        setHeader("Connection", "close"); // HTTP/1.0 default
        buildResponse();
        return ;
    }
    
    setStatus(200);
    setBodyFromFile(file_path);
    setHeader("Connection", "close"); // HTTP/1.0 default connection behavior
    buildResponse();
    return ;
}


void Response::buildPostResponse() {
    setStatus(201);
    setBody("Resource created successfully");
    setHeader("Content-Type", "text/plain");
    setHeader("Connection", "close"); // HTTP/1.0 default
    buildResponse();
    return ;
}

// Build DELETE response
void Response::buildDeleteResponse(const std::string& file_path) {
    if (!fileExists(file_path)) {
        setStatus(404);
        setBody("File not found");
        setHeader("Content-Type", "text/plain");
        setHeader("Connection", "close"); // HTTP/1.0 default
        buildResponse();
        return ;
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
    
    buildResponse();
    return ;
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

std::string Response::getResponse() const {
    return response_str;
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
    
    // For chunked transfer, use Transfer-Encoding instead of Content-Length
    setHeader("Transfer-Encoding", "chunked");
    
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
        // Format as proper HTTP chunk: [hex_size]\r\n[data]\r\n
        std::ostringstream hex_size;
        hex_size << std::hex << bytes_read;
        chunk = hex_size.str() + "\r\n";
        chunk.append(buffer, bytes_read);
        chunk += "\r\n";
        bytes_sent += bytes_read;
        
        // Check if this was the last chunk
        if (file_stream.eof() || bytes_sent >= content_length) {
            // Append final chunk (0\r\n\r\n) to signal end
            chunk += "0\r\n\r\n";
            is_ready = true; // Response is complete
            file_stream.close();
        }
    } else {
        // No more data to read, send terminating chunk
        chunk = "0\r\n\r\n";
        is_ready = true; // Response is complete
        file_stream.close();
    }
    
    return chunk;
}

// Check if response is finished - server uses this to know when to close connection
bool Response::isFinished() const {
    return is_ready;
}


void Response::parseCgiOutput(const std::string &cgi_output) {
    // Split headers from body (if no headers, treat whole output as body)
    size_t header_end = cgi_output.find("\r\n\r\n");
    std::string raw_headers;
    std::string body;

    if (header_end != std::string::npos) {
        raw_headers = cgi_output.substr(0, header_end);
        body = cgi_output.substr(header_end + 4);
    } else {
        // No headers at all -> entire output is body
        body = cgi_output;
    }

    // Parse headers if any exist
    if (!raw_headers.empty()) {
        std::istringstream header_stream(raw_headers);
        std::string line;
        bool status_found = false;

        while (std::getline(header_stream, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);

            size_t colon = line.find(':');
            if (colon == std::string::npos)
                continue; // skip malformed header

            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                value.erase(0, 1);

            if (key == "Status") {
                std::stringstream ss(value);
                int code;
                ss >> code;
                setStatus(code);
                status_found = true;
            } else {
                setHeader(key, value);
            }
        }

        if (!status_found)
            setStatus(200);
    } else {
        // No headers at all → assume normal 200 OK
        setStatus(200);
    }

    // Replace body (always)
    setBody(body);

    // Ensure Content-Length is correct (override if CGI forgot to set)
    std::ostringstream oss;
    oss << body.size();
    setHeader("Content-Length", oss.str());

    // If CGI didn't set Content-Type, fallback to text/html
    if (headers.find("Content-Type") == headers.end())
        setHeader("Content-Type", "text/html");
}

// void Response::parseCgiOutput(const std::string &cgi_output) {
//     // CGI output format is expected as:
//     // Header1: value\r\n
//     // Header2: value\r\n
//     // ...
//     // \r\n
//     // BODY

//     size_t header_end = cgi_output.find("\r\n\r\n");
//     if (header_end == std::string::npos) {
//         // Malformed CGI output → treat as 500 Internal Server Error
//         setStatus(500);
//         setBody("<html><body><h1>500 Internal Server Error</h1><p>Malformed CGI Output</p></body></html>");
//         setHeader("Content-Type", "text/html");
//         return;
//     }

//     std::string raw_headers = cgi_output.substr(0, header_end);
//     std::string body = cgi_output.substr(header_end + 4); // skip \r\n\r\n

//     // Parse headers line by line
//     std::istringstream header_stream(raw_headers);
//     std::string line;
//     bool status_found = false;

//     while (std::getline(header_stream, line)) {
//         if (!line.empty() && line[line.size() - 1] == '\r')  // remove trailing CR
//             line.erase(line.size() - 1);

//         size_t colon = line.find(':');
//         if (colon == std::string::npos)
//             continue; // Skip malformed header lines

//         std::string key = line.substr(0, colon);
//         std::string value = line.substr(colon + 1);
//         while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
//             value.erase(0, 1); // trim leading spaces

//         if (key == "Status") {
//             // "Status: 404 Not Found"
//             std::stringstream ss; ss << value;
//             int code; ss >> code;
//             setStatus(code);
//             status_found = true;
//         } else {
//             setHeader(key, value);
//         }
//     }

//     if (!status_found)
//         setStatus(200); // Default to 200 if CGI didn't send a status

//     setBody(body);

//     // Ensure Content-Length is correct even if CGI forgot to set it
//     std::ostringstream oss;
//     oss << body.size();
//     setHeader("Content-Length", oss.str());

//     // If CGI didn't set Content-Type, fallback to text/html
//     if (headers.find("Content-Type") == headers.end())
//         setHeader("Content-Type", "text/html");
// }
