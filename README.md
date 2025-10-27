# webserv — Response flow detailed README

This README explains, in developer-level detail, how an HTTP response is created and sent in this repository — from the point a TCP connection is accepted in `Connection.cpp` to the response implementations in `src/response/`.

Read this as if you're the author of the server: the document describes the responsibilities of each component, the order of operations, important edge cases, and how to reproduce and test each response path.

## High-level contract

- Input: an accepted TCP socket carrying an HTTP request bytes stream.
- Output: a sequence of bytes written to the socket that represent a valid HTTP response (headers + body). For large bodies, the output may be a chunked transfer encoding stream.
- Error modes: parser errors (400), permission/not found (403/404), server errors (500), CGI errors. Errors must produce an HTTP error response and appropriate error page when available.

## Quick summary of the flow (one-liner steps)

1. Accept connection -> `Connection::Connection` sets up fd and non-blocking mode.
2. Read raw bytes -> request parsing code (`Request` / parsers) fills a `Request` object.
3. Decide the response path: static file, directory/listing, CGI, redirect, error, or method-specific (GET/POST/DELETE).
4. Build response: either a fully-composed response string or a `Response` object that will stream chunked frames.
5. Send bytes on non-blocking socket with partial-write handling until response complete.
6. Close or reuse connection according to keep-alive and server logic.

## Files to inspect (primary)

- `src/server/Connection.cpp` — accepts the socket, sets non-blocking, holds the state for a single client connection (`_fd`, `_request`, `_response`, `_response_obj`, `_isChunkedResponse`, `_responseDone`). Contains `setCGIHeaders()` (wraps CGI output if needed) and helpers.
- `src/server/SendResponse.cpp` — top-level write/send logic (orchestrator). It decides which response builder to call and does the actual socket `write()` calls (handles partial writes and non-blocking semantics).
- `src/Request/*` — request parsers. Key entry: the `Request` object creation/processing which decides CGI, path, headers, and body.
- `src/response/Response.cpp` — response streaming primitives, chunked transfer, and file streaming (`prepareResponse`, `getResponseChunk`, `isFinished`, `buildResponse`, etc.).
- `src/response/GetResponse.cpp`, `PostResponse.cpp`, `DeleteResponse.cpp` — method-specific logic that builds the appropriate response or prepares the `Response` streaming object.
- `src/response/ErrorResponse.cpp` — builds error pages, uses server error-page map.
- `src/response/DirectoryListing.cpp` — builds directory listing HTML for autoindex.
- `src/response/MimeTypes.cpp` — maps file extensions to Content-Type.
- `src/cgi.cpp` — executes CGI processes and provides their output (Connection interfaces with CGI via `_cgiFd` and the connection's read loop).

> Note: some functions and file names above are references; open the files to inspect exact function names and logging statements.

## Detailed flow (expanded)

### 1) Accepting the connection

- Where: `Connection::Connection(int fd, ServerConf& server, const std::string& host, const std::string& port)` in `src/server/Connection.cpp`.
- What it does:
  - Calls `accept()` on the listening socket passed in.
  - Sets the newly accepted socket `_fd` to non-blocking mode using `fcntl` via `setNonBlocking()`.
  - Stores a reference/copy of `ServerConf` to access configuration (document root, locations, error pages, etc.).
  - Initializes CGI state (`_cgiFd = -1`) and timestamps used for timeouts.
- Important details:
  - If `accept()` returns -1 with `EAGAIN`/`EWOULDBLOCK`, the constructor returns early (no connection accepted at this moment).
  - If `fcntl` fails, the constructor throws.

### 2) Reading raw bytes and parsing the request

- Raw bytes are appended into a connection-level buffer (member `_buffer` in `Connection`). The server's read loop calls into the request parser when data indicates the request headers (and body) have been received.
- Request parsing responsibilities:
  - Split the request-line (method, path, version).
  - Parse headers and normalize (Host, Content-Length, Transfer-Encoding, Content-Type, etc.).
  - For POST/multipart/PUT etc., parse the body according to headers.
  - Match the request path against configured locations to determine the root, index files list and whether CGI should be used.
  - Set flags like `isCGI()` and provide a preliminary status code if the request is malformed.
- Result: a `Request` object attached to the `Connection` that contains method, path, headers, body, and matched `LocationConf`.

### 3) Selecting the response path

The send-orchestration routine (the writer) examines the `Request` and decides one of these paths:

- Error/Bad Request: if request parsing returned a non-200 status.
- Redirect: if a configured redirect applies to the matched location/path.
- CGI: if the location or file indicates CGI handling should run.
- GET static file or directory listing.
- POST: upload handling, forms, or CGI-backed POST.
- DELETE: remove a file and produce a response.

This branching logic typically lives in `src/server/SendResponse.cpp` and in each specialized `src/response/*Response.cpp` file.

### 4) Static file GET / directory handling

- Compute the `full_path` by concatenating the selected `document_root` (server or location) and the request URI.
- `stat()` the `full_path`:
  - If it's a regular file:
    - Determine MIME type via `MimeTypes`.
    - If file size is large (threshold is defined in code), prepare a streaming/chunked response using `Response::prepareResponse()` to avoid loading the entire file into memory.
    - If small, call `Response::setBodyFromFile()` and `Response::buildResponse()` to build a single in-memory `_response` string.
  - If it's a directory:
    - Try each `index` filename configured for the location or server. If an index file exists, serve it (repeat the file flow).
    - If no index and `autoindex` is enabled for that location: call the directory-listing builder to create an HTML body.
    - If no index and `autoindex` disabled: return 403 Forbidden.

Edge cases:
- Permissions: if `stat()` or `open()` returns permission denied, return 403.
- Large files: chunked transfer is used to avoid large memory usage.
- Partial reads/writes: non-blocking sockets require correct handling of `EAGAIN`/`EWOULDBLOCK` and partial `write()` returns.

### 5) Building and streaming chunked responses (`Response`)

- `Response::prepareResponse()` opens the file descriptor for the file and composes `cached_headers` that will be sent first. If Transfer-Encoding: chunked is used, the headers include `Transfer-Encoding: chunked` and omit `Content-Length`.
- `Response::getResponseChunk()` returns the next chunk to be written to the socket. Implementation specifics:
  - On the first call, it returns the prepared headers.
  - On subsequent calls, it reads up to a configured chunk size from the file and returns a string that is `hex-length + "\r\n" + data + "\r\n"`.
  - When EOF is reached, it returns the terminating `0\r\n\r\n` chunk and sets an internal finished flag.
- `Response::isFinished()` returns true when the full chunked stream (including the final 0 chunk) has been produced.

### 6) Sending: non-blocking `write()` with partial writes

- The send loop calls `write()` on `_fd` with either a chunk from `Response::getResponseChunk()` (for chunked) or a slice of `_response` string (for non-chunked).
- If `write()` returns a positive number smaller than the data length, the unsent tail is kept and the connection will resume sending on the next writable event.
- If `write()` returns `-1` with `EAGAIN`/`EWOULDBLOCK`, the connection waits for the next writable event.
- When all bytes are written and `Response::isFinished()` is true (or whole `_response` string was sent), the connection sets `_responseDone = true`.

### 7) CGI integration

- The `Request` parser or the send logic determines that a request requires CGI handling (file extension or location). CGI is executed via `src/cgi.cpp`.
- CGI execution flow (typical):
  - Spawn a child process and set up pipes for stdin/stdout.
  - Write request body to CGI stdin when required.
  - Read CGI stdout asynchronously via the `_cgiFd` stored on `Connection`.
  - When CGI stdout is available and fully read, the raw CGI output may either:
    - Already be a full HTTP response (if the CGI script emitted headers + status starting with `HTTP/`), in which case the server may send it as-is.
    - Be just the script output (headers like `Content-Type:` but without `HTTP/1.1`), so `Connection::setCGIHeaders()` wraps it by prepending `HTTP/1.1 200 OK\r\n` and ensuring there is a header/body separator (`\r\n\r\n`) and `Content-Length` if appropriate.
- `Connection::setCGIHeaders()` implementation notes (from provided code fragment):
  - It looks for `\r\n\r\n` or `\n\n` to separate headers and body.
  - If output already begins with `HTTP/` it returns the CGI output unchanged.
  - Otherwise, it constructs an `HTTP/1.1 200 OK` response, appends CGI headers if present, ensures CRLF termination of headers, and appends the body.

Edge cases:
- CGI may produce large output; treat similarly to static file streaming if necessary.
- CGI may produce malformed headers; `setCGIHeaders()` attempts a minimal normalization but you should audit scripts to emit proper headers.

### 8) POST and DELETE

- `POST`:
  - If `POST` is routed to CGI or a handler, the uploading code ensures the body is forwarded to the CGI stdin or processed according to `Content-Type` (multipart, urlencoded, etc.).
  - Success responses are built similarly to GET but often rely on CGI-generated content or custom success pages.
- `DELETE`:
  - Deletes the filesystem object using `unlink()` after permission checks.
  - Returns 200 ok, 403 forbidden, or 404 not found depending on the outcome.

### 9) Error pages and server configuration

- Server configuration (`ServerConf` and `LocationConf`) provide:
  - Document root, allowed methods, index files, autoindex flag, error_page mappings.
- When sending an error, the server tries to use a configured error page (`/www/errors/<status>.html`) or falls back to a minimal HTML body indicating the error code and message.

### 10) State machine and member variables (important connection-level fields)

- `_fd` — client socket fd
- `_buffer` — raw inbound bytes
- `_request` — parsed `Request` object
- `_response` — a full response string used for simple responses and CGI
- `_response_obj` — `Response` instance used for streaming/chunked responses
- `_isChunkedResponse` — toggle to choose streaming path
- `_responseDone` — when true the connection has finished writing the response
- `_cgiFd` — file descriptor for shelling/reading CGI output
- `_time` — for timeout tracking

Understanding how these fields interact (particularly `_response` vs `_response_obj` and `_isChunkedResponse`) is crucial when modifying send logic.

## How to test common scenarios locally

(Assumes you are in the project root, and a `Makefile` is present)

1. Build (if the project uses the top-level `Makefile`):

```bash
make
```

2. Run the server (binary name may be `webserv` or similar produced by the Makefile). If your binary accepts a config path, pass it; otherwise consult `Makefile` or `main.cpp` for usage.

Example guesses (adjust if your build creates a different binary or usage signature):

```bash
# run on default config
./webserv config/config.conf
# or if the binary is `webserv` and accepts port & host args
./webserv 8080
```

3. Request a small static file (should produce a single, non-chunked response):

```bash
curl -v http://localhost:8080/index.html
```

4. Request a large static file to trigger chunked streaming (create a large file inside `www/` first):

```bash
# create a large file for test
dd if=/dev/zero of=www/large_test.bin bs=1M count=10
# request it
curl -v http://localhost:8080/large_test.bin --output /dev/null
```

Watch `stdout` logs of the server for the chunked write behavior, or attach a debugger and step into `Response::getResponseChunk` and `SendResponse` code.

5. Test CGI scripts under `www/cgi-bin/` or `cgi-files/`:

```bash
curl -v http://localhost:8080/cgi-bin/index.py
curl -v http://localhost:8080/cgi-bin/test.php
```

6. Test directory listing (autoindex) by requesting a directory without an index and ensure the matching location has `autoindex` set in the configuration.

7. Test error pages (e.g., 404):

```bash
curl -v http://localhost:8080/nonexistent
```

8. Test POST file upload or multipart handler (if implemented):

```bash
curl -v -F "file=@www/index.html" http://localhost:8080/upload
```

## Debug & development tips (when editing send/response code)

- Add logging at the following strategic points:
  - After `accept()` in `Connection::Connection` (log remote IP/port and fd).
  - After parsing request — log method/path/version and matched location.
  - At branching decision in `SendResponse.cpp` — log chosen path (static, directory, CGI, error, redirect).
  - In `Response::getResponseChunk()` — log when headers are returned, when data chunks are produced, and when final 0 chunk is generated.
  - On `write()` return values — log bytes_written and if the write was partial.
- Validate header correctness (CRLFs) — many HTTP clients are permissive, but strict implementations rely on correct `\r\n` separators.
- Keep non-blocking semantics in mind: all `read()`/`write()` calls must handle `EAGAIN` and partial operations.

## Edge cases and gotchas (author-level notes)

- Be careful when wrapping CGI output: if CGI already emits `HTTP/1.1 200 OK` and custom headers, avoid double-wrapping. `Connection::setCGIHeaders()` attempts to detect `HTTP/` at the start of CGI output.
- CRLF normalization: some scripts emit LF-only newlines. The server should tolerate `\n\n` as a header/body separator but prefer `\r\n` as required by the spec when sending headers.
- Keep-alive handling: ensure `Connection` lifetime is compatible with `Connection::updateTimout()` and that the server reuses the connection only when appropriate.
- Concurrent reads/writes: if a CGI is running and producing output, the server must multiplex socket events and CGI fd events properly. Make sure your poll/epoll logic watches both fds.

## Where to make small improvements (low-risk suggestions)

- Add a unit or integration test for `Response::getResponseChunk()` that writes a temporary file and asserts the sequence: headers, several chunk frames, final 0 chunk.
- Add better validation for CGI headers: prefer to parse `Status:` and `Content-Type` headers and construct an accurate `Content-Length` when possible.
- Make the large-file threshold configurable via `ServerConf`.

## Quick reference and jump list

- Accept & connection: `src/server/Connection.cpp`
- Send orchestration: `src/server/SendResponse.cpp`
- Response streaming: `src/response/Response.cpp`
- GET/POST/DELETE handlers: `src/response/GetResponse.cpp`, `src/response/PostResponse.cpp`, `src/response/DeleteResponse.cpp`
- Error pages: `src/response/ErrorResponse.cpp`
- Directory listing: `src/response/DirectoryListing.cpp`
- MIME types: `src/response/MimeTypes.cpp`
- CGI: `src/cgi.cpp`
- Request parsers: `src/Request/` (multiple files)

## Completion / verification checklist

- [x] `Connection` accepts sockets and sets non-blocking
- [x] Requests are parsed into `Request` objects
- [x] Send logic branches into static, directory, CGI, error, and method-specific handlers
- [x] `Response` supports streaming/chunked responses
- [x] Partial `write()` handling is implemented
- [x] CGI output is normalized by `Connection::setCGIHeaders()` if necessary

If anything above doesn't match your code exactly, open the referenced file and search for the function names mentioned. The exact function names may vary slightly; the README captures the data flow and the locations to inspect.

---

If you want, I can:
- Add a short integration test harness (a script that builds, runs the server, and runs the `curl` checks above), or
- Add a focused unit test for `Response::getResponseChunk()` which will validate chunk formatting and termination.

Tell me which follow-up you'd like (integration script or unit test) and I'll add it next.

## Full implementation snippets (key response-related code)

Below are concise, annotated excerpts of the actual code that implements each major response path. Read each snippet and the short note that follows to understand how the server implements that behavior.

### Connection accept & non-blocking setup
File: `src/server/Connection.cpp` — constructor accepts and configures socket

```cpp
struct sockaddr_in addr;
socklen_t len = sizeof(addr);
_fd = accept(fd, (struct sockaddr*)&addr, &len);
if (_fd == -1){
  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    // No incoming connections right now - just return
    return ;
  } else{
    throw std::runtime_error("Accept failed");
  }
}
_server = server;
_cgiFd = -1;
if (!setNonBlocking())
  throw std::runtime_error("fcntl failed");
updateTimout();
```

Note: the constructor sets `_isChunkedResponse` and `_responseDone` default states elsewhere and stores a copy of `ServerConf` for later routing decisions.

### CGI output normalization (wrap headers when CGI doesn't emit a full HTTP response)
File: `src/server/Connection.cpp` — `setCGIHeaders()`

```cpp
std::ostringstream response;

size_t header_end = _response.find("\r\n\r\n");
if (header_end == std::string::npos) {
  header_end = _response.find("\n\n");
  if (header_end != std::string::npos) {
    header_end += 2;
  }
} else {
  header_end += 4;
}

if (header_end != std::string::npos) {
  std::string headers_part = _response.substr(0, header_end);
  std::string body_part = _response.substr(header_end);

  if (headers_part.find("HTTP/") == 0) {
    return (_response);
  } else {
    response << "HTTP/1.1 200 OK\r\n";
    response << headers_part;
    if (headers_part[headers_part.size() - 1] != '\n') {
      response << "\r\n";
    }
    response << body_part;
  }
} else {
  response << "HTTP/1.1 200 OK\r\n";
  response << "Content-Type: text/html\r\n";
  response << "Content-Length: " << _response.size() << "\r\n";
  response << "\r\n";
  response << _response;
}

return (response.str());
```

Annotation: this function tries to detect whether the CGI output begins with an HTTP status line; if not, it wraps the output with a 200 OK + headers so the rest of the send path can treat it as a full response.

### Send orchestration (non-chunked and chunked cases)
File: `src/server/SendResponse.cpp` — `Connection::writeResponse()`

```cpp
if (_isChunkedResponse) {
  if (!_response_obj.isFinished()) {
    std::string chunk = _response_obj.getResponseChunk();
    if (!chunk.empty()) {
      ssize_t bytes_sent = write(_fd, chunk.c_str(), chunk.length());
      if (bytes_sent == -1) { perror("Chunked write failed"); return false; }
      else if (bytes_sent < (ssize_t)chunk.length()) { /* handle partial */ return false; }
      return true;
    }
  } else {
    _responseDone = true;
    _isChunkedResponse = false;
    return true;
  }
}

// ... decide redirect / CGI / error / POST / DELETE / GET and prepare `_response` or `_response_obj` ...

if (!_isChunkedResponse) {
  ssize_t bytes_sent = write(_fd, _response.c_str() + _responseBytesSent, totalLen - _responseBytesSent);
  updateTimout();
  if (bytes_sent == -1) { return true; }
  _responseBytesSent += bytes_sent;
  if (_responseBytesSent == totalLen) { _responseDone = true; _responseBytesSent = 0; updateTimout(); }
}
```

Notes: the send loop first handles ongoing chunked transfers using `_response_obj` (returns partial sends if write cannot write the whole chunk). For non-chunked responses it writes the `_response` string and tracks partial writes with `_responseBytesSent`.

### Response streaming & chunked transfer
File: `src/response/Response.cpp` — key methods

```cpp
void Response::prepareResponse(const std::string& file_path) {
  file_stream.open(file_path.c_str(), std::ios::binary);
  setStatus(200);
  this->content_length = getFileSize(file_path);
  setHeader("Content-Type", getContentType(file_path));
  setHeader("Transfer-Encoding", "chunked");
  setHeader("Connection", (http_version == "HTTP/1.0") ? "close" : "keep-alive");

  std::ostringstream response;
  response << http_version << " " << status << " " << status_message << CRLF;
  for (it...) response << it->first << ": " << it->second << CRLF;
  response << CRLF;
  this->cached_headers = response.str();
}

std::string Response::getResponseChunk(size_t chunk_size) {
  if (!headers_sent) { headers_sent = true; if (!body.empty() && !file_stream.is_open()) { chunk = cached_headers + body; is_ready = true; return chunk; } chunk = cached_headers; return chunk; }

  if (!file_stream.is_open() || file_stream.eof()) { is_ready = true; return ""; }

  char buffer[BUFFER_SIZE];
  file_stream.read(buffer, actual_chunk_size);
  std::streamsize bytes_read = file_stream.gcount();
  if (bytes_read > 0) {
    std::ostringstream hex_size; hex_size << std::hex << bytes_read;
    chunk = hex_size.str() + "\r\n";
    chunk.append(buffer, bytes_read);
    chunk += "\r\n";
    bytes_sent += bytes_read;
    if (file_stream.eof() || bytes_sent >= content_length) { chunk += "0\r\n\r\n"; is_ready = true; file_stream.close(); }
  } else {
    chunk = "0\r\n\r\n"; is_ready = true; file_stream.close();
  }
  return chunk;
}
```

Explanation: `prepareResponse` builds headers with `Transfer-Encoding: chunked`. `getResponseChunk` returns the headers first, then formatted chunk frames (hex size, data, CRLF) and finally the terminating `0\r\n\r\n` sequence.

### GET handler (static files, directories, autoindex)
File: `src/response/GetResponse.cpp` — `Connection::sendGetResponse()` (excerpt)

```cpp
std::string requested_path = request.getUri();
document_root = this->_server.getRoot();
full_path = document_root + requested_path;
if (stat(full_path.c_str(), &fileStat) == 0) {
  if (S_ISREG(fileStat.st_mode)) {
    size_t file_size = static_cast<size_t>(fileStat.st_size);
    if (file_size > LARGE_FILE_THRESHOLD) {
      _response_obj.prepareResponse(full_path);
      _isChunkedResponse = true;
      _response = "";
      return;
    } else {
      response_obj.setStatus(200);
      response_obj.setBodyFromFile(full_path);
      response_obj.setHeader("Connection", connection_header);
      _response = response_obj.buildResponse();
      _isChunkedResponse = false;
      return;
    }
  } else if (S_ISDIR(fileStat.st_mode)) {
    // try index files, or generate directory listing via generateDirectoryListing()
  }
}
```

Comment: GET chooses chunked streaming for large files by setting `_isChunkedResponse` and preparing `_response_obj`. For small files it composes a full `_response` with headers+body.

### Error response builder (custom pages fallback)
File: `src/response/ErrorResponse.cpp` — `Connection::sendErrorPage()`

```cpp
std::map<std::string, std::string> error_pages = server.getErrorPages();
if (error_pages.find(to_str(code)) != error_pages.end()) {
  std::string full_error_path = "./www/errors/" + error_page;
  if (response_obj.fileExists(full_error_path)) {
    response_obj.setStatus(code);
    response_obj.setBodyFromFile(full_error_path);
    response_obj.setHeader("Content-Type", response_obj.getContentType(full_error_path));
    _response = response_obj.buildResponse();
    _isChunkedResponse = false;
    return;
  }
}
// fallback: inline default HTML body
response_obj.setStatus(code);
response_obj.setBody("<html>...<h1>" + to_str(code) + " " + response_obj.getStatusMessage(code) + "</h1>...");
_response = response_obj.buildResponse();
_isChunkedResponse = false;
```

Note: server config can map error codes to custom pages; otherwise the server returns a minimal HTML body.

### DELETE handler (permissions, path traversal, unlink)
File: `src/response/DeleteResponse.cpp` — key parts

```cpp
// Check allowed methods for location and guarded paths
if (!delete_allowed) { response_obj.setStatus(405); ... _response = response_obj.buildResponse(); return; }
// Protected locations e.g. /cgi-bin, /errors
if (requested_path.find("..") != std::string::npos) {
  response_obj.setStatus(403);
  response_obj.setBody("<html>...Invalid file path...</html>");
  _response = response_obj.buildResponse();
  return;
}
if (unlink(full_path.c_str()) == 0) {
  response_obj.setStatus(200);
  response_obj.setBody("<html>...File successfully deleted...</html>");
  _response = response_obj.buildResponse();
} else {
  int status_code = 500; if (errno == EACCES) status_code = 403; else if (errno == ENOENT) status_code = 404;
  response_obj.setStatus(status_code);
  response_obj.setBody("<html>...error...</html>");
  _response = response_obj.buildResponse();
}
```

### POST handler (simple success / relies on Request processing)
File: `src/response/PostResponse.cpp` — `Connection::sendPostResponse()`

```cpp
response_obj.setStatus(status_code);
response_obj.setHeader("Content-Type", "text/html");
response_obj.setHeader("Connection", connection_header);
response_obj.setBody("<html><body><h1>POST Request Successful</h1>...</body></html>");
_response = response_obj.buildResponse();
_isChunkedResponse = false;
```

### Directory listing generator (autoindex)
File: `src/response/DirectoryListing.cpp` — `generateDirectoryListing()`

```cpp
DIR* dir = opendir(directory_path.c_str());
while ((entry = readdir(dir)) != NULL) {
  std::string entry_name = entry->d_name;
  if (entry_name[0] == '.' && entry_name != "..") continue;
  std::string full_entry_path = directory_path + "/" + entry_name;
  bool is_directory = false;
  if (stat(full_entry_path.c_str(), &entry_stat) == 0) is_directory = S_ISDIR(entry_stat.st_mode);
  entries.push_back(std::make_pair(entry_name, is_directory));
}
// sort and build HTML table with links, sizes and types
```

### MIME type table
File: `src/response/MimeTypes.cpp` — lookup

```cpp
this->mime[".html"] = "text/html";
this->mime[".css"] = "text/css";
this->mime[".js"] = "text/javascript";
// many more entries

std::string MimeTypes::getMimeType(std::string ext) {
  if (mime.find(ext) != mime.end()) return mime[ext];
  return "NONE";
}
```

### Redirect detection & response
File: `src/response/RedirectResponse.cpp`

```cpp
// find best matching location and return its redirect URL if present
for (it...) if (requested_path.find(it->first) == 0 && it->first.length() > best_match.length()) {...}
if (best_location != NULL) {
  std::string redirect_url = best_location->getRedirectUrl();
  if (!redirect_url.empty()) return redirect_url;
}

// Sending redirect
response_obj.setStatus(301);
response_obj.setHeader("Location", redirect_url);
response_obj.setHeader("Content-Type", "text/html");
response_obj.setBody("<html>...Moved Permanently...</html>");
_isChunkedResponse = false;
```

## Wrap-up

This expanded README now includes the exact code excerpts implementing the major response paths and short annotations that explain what each snippet does in the overall flow. If you'd like, I can:

- Insert a small integration script to exercise all these code paths via `curl` and assert HTTP status + snippets in the body.
- Add a unit test for `Response::getResponseChunk()` to validate headers, chunk frames, and final termination.

Which follow-up should I add? (integration script or unit test, or both)
