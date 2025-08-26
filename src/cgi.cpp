#include "cgi.hpp"
#include <sys/time.h> // gettimeofday
ServerConf CGI::_server;
// Using Factory design:

CGI::CGI(){}

CGI::~CGI(){
    std::remove(_cgiFileName.c_str());
}

void CGI::generateCgiFile(){
    timeval ttime; gettimeofday(&ttime, NULL);
    long long microseconds = ttime.tv_sec * 1000000LL + ttime.tv_usec;

    std::stringstream ss; ss << microseconds; ss >> _cgiFileName;
    fd_out = open(_cgiFileName.c_str(), O_CREAT | O_RDWR | O_NONBLOCK);
    // close(fd);
    // std::cout << _cgiFileName << std::endl;
    // file.close();
    // return (fd);

}

std::string CGI::executeCGI(const Request& request, ServerConf& server){
    CGI cgi;
    cgi._server = server;
    // (void)server;
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
     fd_in = request.getCgiFdRead();
    _location = request.getLocation();
    std::map<std::string, LocationConf> locations = _server.getLocations();
    // std::cout << "\n\n\n\n location : " << _location << "\n\n\n\n\n";
    LocationConf conf = locations[_location];
    std::map<std::string, std::string> cgis = conf.getCgi();
    _root = conf.getRoot();
    _execPath = cgis[request.getCgiType()];
    // if (request.getCgiType() == "py")
    //     _execPath = cgis[".py"];
    // else
    //     _execPath = cgis[".php"];


    //  if (request.getCgiType() == "py")
    //     cgiExecPath = "/usr/bin/python3";
    // else cgiExecPath = "";
    _scriptName = request.getUri();        // filesystem path to script (added '.' to use current directory)
    // getRoot();
    _requestMethod = request.getStrMethod()/*FIX!!!*/; //add request.getRequestMethod();     // GET, POST, etc.
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
    generateCgiFile();
    // if (!validPath()){
    //     return (SERVERERROR);
    // }
    printEnvironment();

    lseek(fd_in, 0, SEEK_SET);
    pid_t pid = fork();
    if (pid < 0){
        perror("fork");
        return ("fork error!\n"); // FIX error
    }
    if (pid == 0){
        dup2(fd_in, STDIN_FILENO);
        dup2(fd_out, STDOUT_FILENO);

        close(fd_in);
        close(fd_out);

        std::string ful_path = _root + _scriptName; // fix path
        // should get cgi path from cgi location in config file
        
        // std::cout << " ------------- cgi full path -----\n";
        // std::cout <<  ful_path << std::endl;
        // std::cout << " ------------- exec path -----\n";
        // std::cout <<  _execPath << std::endl;
        char* argv[3];
        argv[0] = const_cast<char*>(_execPath.c_str()); // fix this! it should be argv[3] 
        argv[1] = const_cast<char*>(ful_path.c_str()); // fix this! it should be argv[3] 
        argv[2] = NULL; // should also containe path to exec, py or pl ... 

        execve(_execPath.c_str(), argv, &_envc[0]);
        perror("execve");
        exit(1);
    }
    // if (_requestMethod == "POST" && !_body.empty())
    //     write(fd_out, _body.c_str(), _body.size()); // add protection and fix blocking!
    // close(fd_out);
    
    
    int status;
    waitpid(pid, &status, 0); // check status!
    
    lseek(fd_out, 0, SEEK_SET);
    std::string cgi_output;
    char buffer[1024];
    size_t n; // ssise_t
    while ((n = read(fd_out, buffer, sizeof(buffer))) > 0)
        cgi_output.append(buffer, n);
    close(fd_out);
    // unlink(_cgiFileName);

    std::string response = parseOutput(cgi_output); // add headers here
    // std::cout << "-+-+-+-+-+-+-\n";
    // std::cout << response << "\n";
    // std::cout << "-+-+-+-+-+-+-\n";
    return (response);
}

std::string CGI::parseOutput(std::string& cgi_output){
    // Parse headers/body from CGI output
    // Build full HTTP response
    // fix error: Received HTTP/0.9 when not allowed
    std::ostringstream out;
    out << "HTTP/1.1 200 OK \r\n";
    out << "Content-Length: " << cgi_output.size() << "\r\n";
    out << "Connection: keep-alive\r\n";
    out << "\r\n";
    out << cgi_output;
    std::string response = out.str();
    // std::string response = cgi_output;

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