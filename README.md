# Webserv

Webserv is a small educational HTTP server written in C++ (project for the 1337/42 cursus). It implements an HTTP/1.1-capable server with configuration-driven server blocks, location handling, static file serving, uploads, CGI support (PHP/Python), directory listing (autoindex), and custom error pages.

This README explains the architecture, features, how to build and run the server, and provides step-by-step manual tests (curl examples) so you can verify behavior locally.

## Table of contents

- Project overview
- Features
- Repository layout
- Requirements
- Build
- Run
- Configuration
- Manual tests (curl examples)
- CGI setup
- Notes & troubleshooting

---

## Project overview

Webserv aims to provide a compact, readable HTTP server implementation with the important building blocks of a production server:

- Parsing of server configuration files (listen, root, index, locations, etc.)
- Handling of HTTP requests (request-line, headers, body, chunked transfer encoding)
- Static file serving and content-type resolution
- Directory listing (auto-index)
- File uploads with multipart/form-data
- CGI execution for dynamic scripts (PHP and Python examples included in `www/cgi-bin/`)
- Custom error pages per status code
- Chunked transfer / streaming large files

This server is geared for learning and experimentation rather than as a production-ready web server.

## Features

- Multiple listen addresses and ports
- Location-based routing with per-location settings (root, index, allowed methods, CGI mapping, redirects)
- GET, POST, DELETE (basic) handling
- Multipart/form-data upload support
- CGI execution (pipes, environment setup, reading script output)
- Custom error pages (configurable)
- Directory listing (autoindex) when enabled
- Chunked transfer encoding for large files
- Basic cookie/session header handling used by the example profile pages

## Repository layout (important files)

Top-level:
- `Makefile` - builds the server
- `config/` - sample configuration files (e.g. `default.conf`)
- `includes/` - header files (.hpp)
- `src/` - implementation files (.cpp)
- `www/` - document root served by the example server (static files, CGI scripts, error pages)

Key source files:
- `src/main.cpp` - application entry, loads config and starts server
- `src/server/` - server loop, socket and connection handling
- `src/Request/*` - request parsing and request processing
- `src/response/*` - building responses, mime types, error responses
- `includes/CGI.hpp` & `src/cgi.cpp` - CGI execution
- `config/default.conf` - default config used by the example

## Requirements

- macOS (this repository was prepared on macOS) or a POSIX-compatible OS
- A C++ compiler (g++ or clang++) compatible with the project's Makefile
- `make`
- PHP and/or Python executables if you plan to test PHP/Python CGI scripts (ensure they are executable and accessible by the server)

## Build

From the repository root run:

```sh
make
```

The Makefile builds the `webserv` binary in the project root (or as configured in the Makefile). If your toolchain requires a specific C++ standard or flags, edit the `Makefile` accordingly.

## Run

Default (uses `config/default.conf`):

```sh
./webserv
```

Or pass a custom config file path:

```sh
./webserv config/config.conf
```

The server will read server blocks from the configuration and bind to the configured addresses/ports. The example config typically points document roots to the `www/` folder in the repo.

To stop the server, press Ctrl-C (SIGINT) or send SIGTERM.

## Configuration

Configuration files are located under `config/`. The parser supports directives for:

- `listen` — address[:port] to bind to
- `server_name` — virtual host names
- `root` — filesystem root for the server or a location
- `index` — index files used when a directory is requested
- `error_page` — mapping HTTP status codes to local files
- `location` blocks — per-path overrides: `root`, `auto_index`, `allowed_methods`, `client_max_body_size`, `cgi`, `return` (redirect)

See `config/default.conf` for a working example. The config parser provides error messages for malformed configuration.

## Manual tests (curl examples)

Open a terminal and use `curl` to exercise common functionality. Replace `127.0.0.1:8080` with the host:port shown when the server starts.

1) Simple GET (static file)

```sh
curl -i http://127.0.0.1:8080/index.html
```

Expect: HTTP 200 with the contents of `www/index.html` and a `Content-Type` header.

2) 404 Not Found

```sh
curl -i http://127.0.0.1:8080/nonexistent
```

Expect: HTTP 404 and either the custom error page (if configured) or a simple HTML response.

3) Directory listing (autoindex)

If a location is configured with `auto_index on;`, request a directory path:

```sh
curl -i http://127.0.0.1:8080/some/directory/
```

Expect: generated HTML index listing files and directories.

4) POST upload (multipart/form-data)

```sh
curl -i -X POST -F "file=@/path/to/local/file.txt" http://127.0.0.1:8080/upload
```

Expect: the server stores the uploaded file into the configured upload directory and returns a success response.

5) Chunked / large file streaming

Request a big file (larger than 1MB in the default code path) to exercise chunked streaming behavior:

```sh
curl -i http://127.0.0.1:8080/path/to/large-file.bin
```

Expect: headers indicating chunked transfer or incremental content being received.

6) DELETE (remove file)

```sh
curl -i -X DELETE http://127.0.0.1:8080/path/to/file
```

Expect: 200 OK on success or an error (403/404/405) depending on permissions and allowed methods for the location.

7) CGI test (PHP/Python)

If `www/cgi-bin/index.py` or `www/cgi-bin/index.php` exist and are configured in the `location` `cgi` mapping, run:

```sh
curl -i http://127.0.0.1:8080/cgi-bin/index.py
curl -i http://127.0.0.1:8080/cgi-bin/index.php
```

Expect: The script output (HTTP headers + body) is returned. Make sure the script is readable and the interpreter path in the config is correct (e.g., `/usr/bin/python3`, `/usr/bin/php`).

8) Redirects

If a location has a `return 301 /new-path;` directive, requesting the old path should return a 301 with a `Location` header:

```sh
curl -i http://127.0.0.1:8080/old-path
```

Expect: `HTTP/1.1 301` and `Location: /new-path` header.

9) Profile/login example (header-based session)

The example `www/profile/*` pages use headers to populate user data when `Request::loggedIn` is set by example logic. A GET to those pages will include custom headers like `X-User-Username` if the server has session info.

## CGI setup

- Ensure the CGI interpreter path (PHP/Python) is mapped in the configuration (per-location `cgi` directives).
- Make the scripts in `www/cgi-bin/` readable and executable by the user running the server if needed.
- The server sets standard CGI environment variables and passes the request body via a pipe for POST requests.

Example `config` snippet for CGI mapping:

```
location /cgi-bin/ {
    root ./www;
    cgi .py /usr/bin/python3;
    cgi .php /usr/bin/php;
}
```

Adjust paths to the interpreters on your machine.

## Notes & troubleshooting

- If the server fails to bind ports, ensure the configured port is free and you have permission to bind (ports <1024 require root privileges).
- If CGI fails with "interpreter not found" or similar, verify the `cgi` mapping and that the interpreter binary is executable.
- If file uploads fail, check upload directory permissions and `client_max_body_size` limits in the config.
- For debugging, you may add temporary `std::cout` statements or run under `lldb/gdb` to inspect behavior.

## Contribution & extensions

This repository is a learning project. Suggested improvements:

- Add unit/integration tests
- Harden input parsing and security checks
- Add TLS support (via OpenSSL)
- Add logging subsystem and configurable log levels

## Team

This project was developed by a dedicated team — big thanks to all contributors:

- [Houssam Mrabet](https://github.com/HoussamMrabet)
- [Mohammed El Hamdaoui](https://github.com/mel-hamd)
- [Chorouk Masnaoui](https://github.com/Cho-r-ouk)

We appreciate everyone's work and collaboration on this project.