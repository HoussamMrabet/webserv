#include "cgi.hpp"

// Using Factory design:

CGI::CGI(){}

std::string CGI::executeCGI(const Request& request){
    CGI cgi;
    cgi.importData(request);
    std::string response = cgi.runCGI();
    return (response);
}

void CGI::setQueryString(){
    std::string script_path;
    size_t pos =_scriptName.find('?');
    if (pos != std::string::npos){
        script_path =_scriptName.substr(0, pos);
        _queryString =_scriptName.substr(pos + 1);
       _scriptName = script_path;
    }
}

void CGI::setContentLenght(){
    if (_requestMethod == "POST" && !_body.empty()){
        std::stringstream ss;
        ss << _body.size();
        ss >> _contentLenght;
    }
}

void CGI::set_HTTP_Header(){
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        std::string envName = it->first;
        if (it->first == "Content-Type") _contentType = it->second;
        for (size_t i = 0; i < envName.size(); ++i)
            envName[i] = envName[i] == '-'? '_': toupper(envName[i]);
        _envs.push_back("HTTP_" + envName + "=" + it->second);
    }
}

void CGI::importData(const Request& request){
     // env variables:
    _scriptName = request.getUri();        // filesystem path to script (added '.' to use current directory)
    _requestMethod = "GET"/*FIX!!!*/; //add request.getRequestMethod();     // GET, POST, etc.
    _queryString = "";       // stuff after '?' if it exists
    _body = request.getBody();              // POST body (if any)
    _contentLenght = "";     // _body.size()
    _contentType = "";       // from the headers map (should be!!)
    _remoteAddr = request.getHost();        // remote client IP
    _headers = request.getHeaders(); // HTTP headers
    setQueryString();
    setContentLenght();
    set_HTTP_Header();
    _envs.push_back("SCRIPT_NAME=" + _scriptName); // SCRIPT_NAME - URL path to script
    _envs.push_back("SCRIPT_FILENAME=" + _scriptName/* FIX!!!*/); // SCRIPT_FILENAME - full path on filesystem
    _envs.push_back("REQUEST_METHOD=" + _requestMethod);
    _envs.push_back("QUERY_STRING=" + _queryString);
    _envs.push_back("CONTENT_LENGTH=" + _contentLenght);
    _envs.push_back("CONTENT_TYPE=" + _contentType); // CONTENT_TYPE (if present)
    _envs.push_back("REMOTE_ADDR=" + _remoteAddr); // REMOTE_ADDR - client IP
    _envs.push_back("SERVER_PROTOCOL=HTTP/1.1"); // SERVER_PROTOCOL
    _envs.push_back("SERVER_SOFTWARE=webserv/0.1"); // SERVER_SOFTWARE - your server name/version
    _envs.push_back("GATEWAY_INTERFACE=CGI/1.1"); // GATEWAY_INTERFACE - CGI version
    // add rest of env vars
    for (size_t i = 0; i < _envs.size(); ++i)
        _envc.push_back(const_cast<char*>(_envs[i].c_str()));
    _envc.push_back(NULL);
}

std::string CGI::runCGI(){
    // signal(SIGPIPE, SIG_IGN); // trying to catch broken pipe signal after nonblok
    if (!validPath()){
        return (SERVERERROR);
    }
    printEnvironment();
    int pipe_out[2]; // child writes into pipe_out[1] (execve output) -> parent reads from pipe_out[0]
    int pipe_in[2]; // parent writes into pipe_in[1] (POST body) -> child reads from pipe_in[0]
    if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1){
        perror("pipe");
        return ("pipe error!\n"); //FIX error: "HTTP/1.1 500 Internal Server Error\r\n\r\n"
    }
    // // adding non blocking causes broken pipe error!!!
    pid_t pid = fork();
    if (pid < 0){
        perror("fork");
        return ("fork error!\n"); // FIX error
    }
    if (pid == 0){
        dup2(pipe_out[1], STDOUT_FILENO);
        dup2(pipe_in[0], STDIN_FILENO);

        close(pipe_out[0]);
        close(pipe_out[1]);
        close(pipe_in[0]);
        close(pipe_in[1]);

        char* argv[2];
        argv[0] = const_cast<char*>(_scriptName.c_str()); // fix this! it should be argv[3] 
        argv[1] = NULL; // should also containe path to exec, py or pl ... 

        execve(argv[0], argv, &_envc[0]);
        perror("execve");
        exit(1);
    }
    close(pipe_out[1]);
    close(pipe_in[0]);

    if (_requestMethod == "POST" && !_body.empty())
        write(pipe_in[1], _body.c_str(), _body.size()); // add protection and fix blocking!
    close(pipe_in[1]);


    std::string cgi_output;
    char buffer[1024];
    size_t n; // ssise_t
    while ((n = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
        cgi_output.append(buffer, n);
    close(pipe_out[0]);


    int status;
    waitpid(pid, &status, 0); // check status!

    std::string response = parseOutput(cgi_output);
    return (response);
}

std::string CGI::parseOutput(std::string& cgi_output){
    // Parse headers/body from CGI output
    // Build full HTTP response
    // fix error: Received HTTP/0.9 when not allowed

    std::string response = cgi_output;
    return (response);
}

void CGI::printEnvironment(){ // to remove later
    std::cout << "...........Environment...............\n"; 
    // std::cout << "--- > env size = " << _envs.size() << std::endl;
    for (size_t i = 0; i < _envs.size(); i++)
        std::cout << _envs[i] << std::endl;
    std::cout << "...................................\n"; 
}


bool CGI::setToNonBlocking(int/* pipe_fd*/){
    // set pipe_fds used in the parent process to non blocking
    // to avoid blocking read and write
    // fcntl() ?
    return (true);
}

bool CGI::validPath(){
    // no path traversal ../?
    // file is executable: access(_scriptName.c_str(), X_OK) ?
    // no need to check for valid path if execve returns error in all error cases?
    // Check if it's within the allowed CGI directory if there is one??
    return (true);
}