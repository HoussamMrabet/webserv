# Chunked Transfer Encoding Implementation

## Overview

This webserver implements efficient file serving using automatic chunked transfer encoding for large files. The system automatically detects file sizes and chooses the optimal transfer method to prevent memory exhaustion and improve performance.

## Architecture

### Core Components

1. **Connection Class** (`src/server/Connection.cpp`, `includes/Connection.hpp`)
   - Manages client connections and response handling
   - Decides between regular and chunked transfer based on file size
   - Handles the chunked transfer state machine

2. **Response Class** (`src/response/Response.cpp`, `includes/Response.hpp`)
   - Provides chunked transfer functionality
   - Manages file streaming with configurable buffer sizes
   - Builds proper HTTP responses with appropriate headers

## How It Works

### File Size Detection

```cpp
#define LARGE_FILE_THRESHOLD 1048576  // 1MB
```

- Files **≤ 1MB**: Loaded entirely into memory (regular response)
- Files **> 1MB**: Streamed using chunked transfer

### Transfer Decision Flow

```
Request for file → Check file size → Choose transfer method:
                                   ├── Small file: Regular response
                                   └── Large file: Chunked response
```

### Connection State Management

The `Connection` class maintains several key state variables:

```cpp
Response _response_obj;      // For chunked responses
std::string _response;       // For simple responses  
bool _isChunkedResponse;     // Transfer mode flag
```

### Chunked Transfer Process

1. **Preparation Phase**
   ```cpp
   _response_obj.prepareResponse(file_path);
   _isChunkedResponse = true;
   ```

2. **Response Loop**
   ```cpp
   while (!_response_obj.isFinished()) {
       std::string chunk = _response_obj.getResponseChunk();
       write(socket, chunk.data(), chunk.size());
   }
   ```

3. **Completion**
   ```cpp
   if (_response_obj.isFinished()) {
       _responseDone = true;
       _isChunkedResponse = false;
   }
   ```

## Response Class API

### Chunked Transfer Methods

#### `prepareResponse(const std::string& file_path)`
- Initializes chunked transfer for a file
- Sets up file stream and HTTP headers
- Handles error cases (file not found, permission issues)

#### `getResponseChunk(size_t chunk_size = BUFFER_SIZE)`
- Returns next chunk of data
- First call returns HTTP headers
- Subsequent calls return file content chunks
- Returns empty string when finished

#### `isFinished() const`
- Returns `true` when entire file has been sent
- Used by Connection to determine when to close

### Example Usage

```cpp
// For large files
response_obj.prepareResponse("/path/to/large/file.zip");
while (!response_obj.isFinished()) {
    std::string chunk = response_obj.getResponseChunk();
    if (!chunk.empty()) {
        send_to_client(chunk);
    }
}
```

## HTTP Response Format

### Small Files (Regular Response)
```http
HTTP/1.1 200 OK
Connection: keep-alive
Content-Length: 1024
Content-Type: text/html

[entire file content]
```

### Large Files (Chunked Response)
```http
HTTP/1.1 200 OK
Connection: keep-alive
Content-Length: 5242880
Content-Type: application/octet-stream

[first chunk of file content]
```

Then subsequent chunks are sent in separate write operations.

## Configuration

### Buffer Size
```cpp
#define BUFFER_SIZE 4096  // 4KB chunks
```

### File Size Threshold
```cpp
#define LARGE_FILE_THRESHOLD 1048576  // 1MB
```

## Supported File Types

The system automatically detects MIME types and handles all file types:
- Text files (HTML, CSS, JS, etc.)
- Images (PNG, JPG, GIF, etc.)
- Videos and large media files
- Archives and executables

## Error Handling

### File Not Found
```cpp
if (!fileExists(file_path)) {
    setStatus(404);
    setBody("File not found");
    return;
}
```

### Permission Denied
```cpp
if (!file_stream.is_open()) {
    setStatus(500);
    setBody("Cannot open file");
    return;
}
```

### Connection Issues
- Automatic cleanup of file streams
- Proper connection state reset
- Error logging with `perror()`

## Performance Benefits

1. **Memory Efficiency**
   - Large files are not loaded entirely into RAM
   - Constant memory usage regardless of file size
   - Prevents server crashes on large file requests

2. **Scalability**
   - Multiple large file transfers can run simultaneously
   - No blocking on large file operations
   - Configurable buffer sizes for optimization

3. **Client Compatibility**
   - Standard HTTP/1.1 responses
   - Works with all HTTP clients (browsers, curl, etc.)
   - Proper Content-Length headers

## Integration with Existing Features

### Redirect Support
```cpp
// Redirects work with both transfer methods
if (!redirect_url.empty()) {
    _response = sendRedirectResponse(_request, redirect_url, _server);
    _isChunkedResponse = false;  // Redirects use regular responses
}
```

### Error Pages
```cpp
// Error responses always use regular transfer
sendErrorPage(_request, status_code, _server);
_isChunkedResponse = false;
```

### Directory Listings
- Index files are also subject to size checking
- Large index files use chunked transfer
- Small index files use regular transfer

## Testing

### Create Large Test File
```bash
# Create 5MB test file
dd if=/dev/zero of=www/large_file.bin bs=1M count=5
```

### Test with curl
```bash
# Test chunked transfer
curl -v http://127.0.0.1:8080/large_file.bin

# Test regular transfer  
curl -v http://127.0.0.1:8080/index.html
```

### Expected Output
```
File size: 5242880 bytes
Using chunked transfer for large file
Sent chunk of size: 4096
Sent chunk of size: 4096
...
Chunked response complete
```

## Future Enhancements

1. **True HTTP Chunked Encoding**
   - Implement `Transfer-Encoding: chunked` header
   - Send hex chunk sizes before each chunk
   - Add final `0\r\n\r\n` terminator

2. **Compression Support**
   - Gzip compression for text files
   - Automatic compression detection
   - `Content-Encoding` headers

3. **Range Requests**
   - Support for `Range: bytes=` headers
   - Partial content responses (206 status)
   - Resume download capability

4. **Configurable Thresholds**
   - Runtime configuration of file size threshold
   - Per-location chunk size settings
   - Adaptive chunk sizes based on connection speed

## Troubleshooting

### Common Issues

1. **"Chunked write failed"**
   - Check client connection status
   - Verify file permissions
   - Monitor system file descriptor limits

2. **Memory Usage Still High**
   - Verify `LARGE_FILE_THRESHOLD` setting
   - Check for memory leaks in file stream handling
   - Monitor buffer sizes

3. **Slow Transfer Speeds**
   - Adjust `BUFFER_SIZE` (4KB default)
   - Check disk I/O performance
   - Monitor network conditions

### Debug Output
Enable verbose logging to see transfer decisions:
```
File size: 2097152 bytes
Using chunked transfer for large file
Sent chunk of size: 4096
Sent chunk of size: 4096
Chunked response complete
```

## Conclusion

This chunked transfer implementation provides robust, memory-efficient file serving for web servers handling files of any size. The automatic detection and seamless fallback ensure optimal performance for both small and large file transfers while maintaining full HTTP compliance.
