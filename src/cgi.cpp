#include "cgi.hpp"
#include <sys/time.h>

CGI::CGI(): _fd_in(-1), _fd_out(-1), _execDone(false), _readDone(false){}

CGI::~CGI(){
    if (_fd_in != -1)
        close(_fd_in);
    if (_fd_out != -1)
        close(_fd_out);
}
std::string CGI::executeCGI(Request& request, ServerConf& server){
    if (_readDone || _execDone)
        return (_output);

    _server = server;
    importData(request);
    
    int stdout_pipe[2];
    if (pipe(stdout_pipe) < 0)
        throw std::runtime_error("Pipe failed");

    _fd_out = stdout_pipe[0];
    pid_t pid = fork();

    if (pid < 0) {
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        throw std::runtime_error("Fork failed");
    }
    
    if (pid == 0){
        // Child process
        if (_fd_in != -1){
            lseek(_fd_in, 0, SEEK_SET);
            dup2(_fd_in, STDIN_FILENO);
            close(_fd_in);
        }
        close(stdout_pipe[0]); // close read end
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);
        close(stdout_pipe[1]);

        char* argv[3];
        argv[0] = const_cast<char*>(_execPath.c_str());
        argv[1] = const_cast<char*>(_scriptFileName.c_str());
        argv[2] = NULL;

        execve(_execPath.c_str(), argv, &_envc[0]);
        perror("execve");
        exit(1);
    }
    
    // Parent process
    close(stdout_pipe[1]); // close write end
    _pid = pid; // Store PID for later cleanup
    
    if (!setToNonBlocking())
        throw std::runtime_error("fcntl failed");

    // DON'T wait here - let it run asynchronously
    _execDone = true;
    return (_output);
}

std::string CGI::readOutput(){
    char buffer[4096];
    ssize_t n;

    if (!_execDone || _readDone) 
        return (_output);
    
    // Non-blocking read
    n = read(_fd_out, buffer, sizeof(buffer));
    
    if (n > 0) {
        _output.append(buffer, n);
    }
    else if (n == 0) {
        // EOF - CGI finished
        _readDone = true;
        
        // Now we can wait for the process
        int status;
        if (waitpid(_pid, &status, WNOHANG) > 0) {
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                close(_fd_in);
                throw std::runtime_error("CGI script failed");
            }
        }
        
        close(_fd_in);
    }
    else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        // Real error (not just "would block")
        close(_fd_in);
        throw std::runtime_error("Read failed");
    }
    // If errno is EAGAIN/EWOULDBLOCK, just return what we have so far
    
    return (_output);
}

// Add to cgi.hpp:
// pid_t _pid;  // Store the child process PID

void CGI::setContentLenght(){
    if (_requestMethod == "POST" && !_body.empty()){
        std::stringstream ss;
        ss << _body.size();
        _contentLenght = ss.str();
    }
}

void CGI::set_HTTP_Header(){
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); 
        it != _headers.end(); ++it){
        std::string envName = it->first;
        if (it->first == "content-type"){
            _contentType = it->second;
            continue;
        }
        if (it->first == "content-length")
            continue;
        for (size_t i = 0; i < envName.size(); ++i)
            envName[i] = (envName[i] == '-') ? '_' : toupper(envName[i]);
        _envs.push_back("HTTP_" + envName + "=" + it->second);
    }
}

void CGI::importData(Request& request){
    _execDone = false;
    _readDone = false;
    _fd_in = request.getCgiFdRead();
    _location = request.getLocation();
    std::map<std::string, LocationConf> locations = _server.getLocations();
    LocationConf conf = locations[_location];
    std::map<std::string, std::string> cgis = conf.getCgi();
    _root = conf.getRoot();
    _root = _root.empty()? _server.getRoot(): _root; // if no root inside location get global root!!!
    _scriptName = request.getFullUri();
    _scriptFileName = request.getFullPath(); // should be set after _scriptName _location and _root
    _execPath = cgis[request.getCgiType()];
    // std::cout << "exec path = " << _execPath << std::endl;
    _queryString = request.getUriQueries();
    _requestMethod = request.getStrMethod();
    _body = request.getBody();
    _remoteAddr = request.getHost();
    _headers = request.getHeaders();

    if (!validExec()){
        // close(_fd_in);
        // close(_fd_out);
        throw std::runtime_error("CGI interpreter not found");
    }
    if (!validPath()){
        // close(_fd_in);
        // close(_fd_out);
        throw std::runtime_error("Invalid CGI script path");
    }

    setContentLenght(); // check output and throw error
    set_HTTP_Header();
    _envs.push_back("SCRIPT_NAME=" + _scriptName);
    _envs.push_back("SCRIPT_FILENAME=" + _scriptFileName);
    _envs.push_back("REQUEST_METHOD=" + _requestMethod);
    _envs.push_back("QUERY_STRING=" + _queryString);
    _envs.push_back("CONTENT_LENGTH=" + _contentLenght);
    _envs.push_back("CONTENT_TYPE=" + _contentType);
    _envs.push_back("REMOTE_ADDR=" + _remoteAddr);
    _envs.push_back("SERVER_PROTOCOL=HTTP/1.1");
    _envs.push_back("SERVER_SOFTWARE=webserv/1.0");
    _envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
    // _envc.clear();

    for (size_t i = 0; i < _envs.size(); ++i)
        _envc.push_back(const_cast<char*>(_envs[i].c_str()));
    _envc.push_back(NULL);
}

// std::string CGI::readOutput(){
//     char buffer[4096];
//     int n;

//     if (!_execDone || _readDone) 
//         return (_output);
//     // int read_counter = 0;
//     n = read(_fd_out, buffer, sizeof(buffer));
//     if (n > 0)
//         _output.append(buffer, n);
//     // std::cout << "n = " << n << std::endl;
//     else if (n == 0){ // change to = 0 and check for -1 
//         _readDone = true;
//         close(_fd_in);
//         // close(_fd_out);
//     }
//     else{
//         // _readDone = true;
//         close(_fd_in);
//         // close(_fd_out);
//         throw std::runtime_error("Read failed");
//     } 
//     // throw exception!!
//     return (_output);
// }

void CGI::printEnvironment(){
    std::cout << "...........Environment...............\n"; 
    for (size_t i = 0; i < _envc.size() - 1; i++){
        if (_envc[i]){
            std::cout << _envc[i] << std::endl;
        }
    }
    std::cout << "...................................\n"; 
}

bool CGI::setToNonBlocking(){
    return ((fcntl(_fd_out, F_SETFL, O_NONBLOCK) != -1));
}

bool CGI::validPath(){
    if (access(_scriptFileName.c_str(), F_OK) != 0) // change file permission and test 
        return (false);
    if (access(_scriptFileName.c_str(), R_OK) != 0)
        return (false);
    return (true);
}

bool CGI::validExec(){
    if (access(_execPath.c_str(), X_OK) != 0)
        return (false);
    return (true);
}

int CGI::getFd(){ return (_fd_out);}

bool CGI::readDone(){ return (_readDone);}

bool CGI::execDone(){ return (_execDone);}
