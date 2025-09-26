#include "cgi.hpp"
#include <sys/time.h>

ServerConf CGI::_server;

CGI::CGI(): _fd_in(-1), _fd_out(-1), _execDone(false), _readDone(false) {}

CGI::~CGI(){
    std::cout << "+++++++++++++ CGI destructor!!!!!!!!!!!!!\n";
    if (_fd_in != -1)
        close(_fd_in);
    // if (_fd_out != -1)
    //     close(_fd_out);
    // if (!_cgiFileName.empty())
    //     std::remove(_cgiFileName.c_str());
}

// void CGI::generateCgiFile(){
//     timeval ttime; 
//     gettimeofday(&ttime, NULL);
//     long long microseconds = ttime.tv_sec * 1000000LL + ttime.tv_usec;

//     std::stringstream ss; 
//     ss << "cgi_" << microseconds;
//     // ss << "/tmp/cgi_" << microseconds;
//     _cgiFileName = ss.str();
    
//     _fd_out = open(_cgiFileName.c_str(), O_CREAT | O_RDWR, 0644);
//     // _fd_out = open(_cgiFileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
//     if (_fd_out == -1) {
//         perror("open cgi file");
//         throw std::runtime_error("Failed to create CGI output file");
//     }
// }

int CGI::getFd() const { return (_fd_out);}

// void CGI::setFd(int fd){ _fd_in = fd; }

std::string CGI::executeCGI(const Request& request, ServerConf& server){
    try {
        // std::cout << G"-------- Still here1!!!! ";
        // std::cout << B"\n";
        if (!_execDone){
            // CGI cgi;
            _server = server;
            importData(request);
            // std::cout << C"-------- tring to execute cgi";
            // std::cout << B"\n";
            
            runCGI();
            return ("");
        }
        else {
            return (sendResponse());
        }
        // return (runCGI());
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return (SERVERERROR);
    }
}

// void CGI::setContentLenght(){
//     if (_requestMethod == "POST" && !_body.empty()){
//         std::stringstream ss;
//         ss << _body.size();
//         _contentLenght = ss.str();
//     } else {
//         _contentLenght = "";
//     }
// }

void CGI::set_HTTP_Header(){
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); 
         it != _headers.end(); ++it) {
        std::string envName = it->first;
        if (it->first == "content-type"){
            _contentType = it->second;
            continue;
        }
        if (it->first == "content-length"){
            // std::cout << "++++++++++++++++ content-lenght = " << it->second << std::endl;
            // _contentType = it->second;
            continue;
        }
        
        for (size_t i = 0; i < envName.size(); ++i) {
            envName[i] = (envName[i] == '-') ? '_' : toupper(envName[i]);
        }
        _envs.push_back("HTTP_" + envName + "=" + it->second);
    }
}

std::string CGI::setPath(){ // to check !!! 
    size_t pos = _scriptName.find(_location);
    std::string path;
    if (pos != std::string::npos)
        pos += _location.size();
    path = _scriptName.substr(pos, _scriptName.size());
    // std::cout << M"data: pos " << pos << " path " << path << " root " << _root;
    path = _root + path;
    // std::cout << " path " << path << " location " << _location  << std::endl;

    return (path);
}

void CGI::importData(const Request& request){
    _execDone = false;
    _execDone = false;
    _fd_in = request.getCgiFdRead();
    _location = request.getLocation();
    CHOROUK && std::cout << C"*********** location is  " << _location << std::endl;
    
    std::map<std::string, LocationConf> locations = _server.getLocations();
    LocationConf conf = locations[_location];
    std::map<std::string, std::string> cgis = conf.getCgi();
    
    /******************* get root ******************/
    // if ()
    _root = conf.getRoot(); // if no root inside location get global root!!!
    CHOROUK && std::cout << C"*********** root is  " << _root << std::endl;
    CHOROUK && std::cout << C"*********** file is  " << request.getUriFileName() << std::endl;
    /******************* get root ******************/
    _scriptName = request.getUri();
    
    // Handle directory requests (ending with /) - use index file
    if (_scriptName.empty() || _scriptName[_scriptName.size() - 1] == '/') {
        std::vector<std::string> indexFiles = conf.getIndex();
        if (!indexFiles.empty()) {
            _scriptName += indexFiles[0]; // Use first index file
        }
    }
    // std::string name = request.getUriFileName();
    _scriptFileName = "." + setPath(); // should be set after _scriptName _location and _root
    CHOROUK && std::cout << C"*********** _scriptName is  " << _scriptName << std::endl;
    CHOROUK && std::cout << C"*********** _scriptFILEName is  " << _scriptFileName << std::endl;
    // CHOROUK && std::cout << C"*********** name is  " << name << std::endl;
    // _scriptFileName = _root + _scriptName;
    CHOROUK && std::cout << M"----> " << _scriptFileName;
    CHOROUK && std::cout << std::endl;
    _execPath = cgis[request.getCgiType()];
    _queryString = request.getUriQueries();
    _requestMethod = request.getStrMethod();
    _body = request.getBody();
    _remoteAddr = request.getHost();
    _headers = request.getHeaders();

    if (_execPath.empty()) {
        // std::cout << G"-------- Im the problem 1!!!!! ";
        // std::cout << B"\n";
        throw std::runtime_error("CGI interpreter not found");
    }
    
    if (!validPath()) {
        // std::cout << G"-------- Im the problem 2!!!!! ";
        // std::cout << B"\n";
        throw std::runtime_error("Invalid CGI script path");
    }

    // setContentLenght();
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
    
    _envc.clear();
    for (size_t i = 0; i < _envs.size(); ++i) {
        _envc.push_back(const_cast<char*>(_envs[i].c_str()));
    }
    _envc.push_back(NULL);
}

void CGI::runCGI(){
    // std::cout << G"-------- Still here0!!!! ";
    // std::cout << B"\n";
    // generateCgiFile();
    // printEnvironment();//////////     TO print env
    // if (_fd != -1) {
    //     lseek(_fd, 0, SEEK_SET);
    // }
    // int pipefd[2];
    // if (pipe(pipefd) == -1) {
    //     perror("pipe");
    //     return;
    // }
    // pid_t pid = fork();
    // if (pid < 0){
    //     // std::cout << G"-------- Im the problem 3!!!!! ";
    //     // std::cout << B"\n";
    //     throw std::runtime_error("Fork failed");
    // }
    
    // if (pid == 0) {
    //     if (_fd_in != -1) {
    //         lseek(_fd_in, 0, SEEK_SET);
    //         dup2(_fd_in, STDIN_FILENO);
    //         // close(_fd_in);
    //     }
    //     // else {
    //     //     int dev_null = open("/dev/null", O_RDONLY);
    //     //     if (dev_null != -1) {
    //     //         dup2(dev_null, STDIN_FILENO);
    //     //         close(dev_null);
    //     //     }
    //     // }
        
    //     // dup2(_fd_in, STDOUT_FILENO);
    //     // // dup2(_fd_in_out, STDOUT_FILENO);
    //     // dup2(STDOUT_FILENO, STDERR_FILENO);
        
    //     dup2(pipefd[1], STDOUT_FILENO);
    //     dup2(pipefd[1], STDERR_FILENO);
    //     close(pipefd[1]);

    //     if (_fd_in != -1) close(_fd_in);
    //     // close(_fd_out);

        char* argv[3];
        argv[0] = const_cast<char*>(_execPath.c_str());
        argv[1] = const_cast<char*>(_scriptFileName.c_str());
        argv[2] = NULL;

        // std::cout << "+++++++++++++++EXECVE+++++++++++++\n";
        execve(_execPath.c_str(), argv, &_envc[0]);
        perror("execve");
        exit(1); // remove!! throw exception
    // }
    
    // int status;
    // waitpid(pid, &status, 0);
    
    // if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    //     // std::cout << G"-------- Im the problem 4!!!!! ";
    //     // std::cout << B"\n";
    //     throw std::runtime_error("CGI script failed");
    // }
    // _execDone = true;
    // _fd_out = pipefd[1];
    // // // _done = true;
    // // lseek(_fd, 0, SEEK_SET);
    // // // lseek(_fd_out, 0, SEEK_SET);
    // // std::string cgi_output;
    // // char buffer[4096];
    // // ssize_t n;

    // // add _fd and fd_out to poll and manage all without while loop!! 
    
    // int read_counter = 0;
    // while ((n = read(_fd, buffer, sizeof(buffer))) > 0) {
    //     std::cout << "read counter = " << ++read_counter << std::endl;
    //     cgi_output.append(buffer, n);
    // }
    // cgi_output = parseOutput(cgi_output);
    // // std::cout << " ########## CGI ###########\n";
    // // std::cout << cgi_output << std::endl;
    // // std::cout << " ##########################\n";



    // return (cgi_output);
}

std::string CGI::sendResponse(){

        // _done = true;
    lseek(_fd_out, 0, SEEK_SET);
    // lseek(_fd_out, 0, SEEK_SET);
    std::string cgi_output;
    char buffer[4096];
    ssize_t n;

    // add _fd and fd_out to poll and manage all without while loop!! 
    
    int read_counter = 0;
    n = read(_fd_out, buffer, sizeof(buffer));
    if (n > 0) {
        std::cout << "read counter = " << ++read_counter << std::endl;
        cgi_output.append(buffer, n);
    }
    else _readDone = true;
    cgi_output = parseOutput(cgi_output);
    std::cout << " ########## CGI ###########\n";
    std::cout << cgi_output << std::endl;
    std::cout << " ##########################\n";



    return (cgi_output);
}

bool CGI::readDone() const{ return (_readDone);}
bool CGI::execDone() const{ return (_execDone);}
// bool CGI::isDone() { return (_execDone && _readDone);}

std::string CGI::parseOutput(std::string& cgi_output){
    std::ostringstream response;
    
    size_t header_end = cgi_output.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        header_end = cgi_output.find("\n\n");
        if (header_end != std::string::npos) {
            header_end += 2;
        }
    } else {
        header_end += 4;
    }
    
    if (header_end != std::string::npos) {
        std::string headers_part = cgi_output.substr(0, header_end);
        std::string body_part = cgi_output.substr(header_end);
        
        if (headers_part.find("HTTP/") == 0) {
            return (cgi_output);
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
        response << "Content-Length: " << cgi_output.size() << "\r\n";
        response << "\r\n";
        response << cgi_output;
    }
    
    return (response.str());
}

void CGI::printEnvironment(){
    std::cout << "...........Environment...............\n"; 
    for (size_t i = 0; i < _envc.size() - 1; i++) {
        if (_envc[i]) {
            std::cout << _envc[i] << std::endl;
        }
    }
    std::cout << "...................................\n"; 
}

// bool CGI::setToNonBlocking(int fd){
//     int flags = fcntl(fd, F_GETFL);
//     if (flags == -1) return (false);
//     return ((fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1));
// }

bool CGI::validPath(){
    // std::cout << G"path is " << _scriptFileName;
    // std::cout << B"\n";
    if (access(_scriptFileName.c_str(), F_OK) != 0){

        // std::cout << G"I fail!!!! 1" << _scriptFileName;
        // std::cout << B"\n";
        return (false);
    } 
    if (access(_scriptFileName.c_str(), R_OK) != 0){
        // std::cout << G"I fail!!!! 2" << _scriptFileName;
        // std::cout << B"\n";
        return (false);
    } 
    if (access(_execPath.c_str(), X_OK) != 0){
        // std::cout << G"I fail!!!! 3" << _execPath;
        // std::cout << G"I fail!!!! 3" << _scriptFileName;
        // std::cout << B"\n";
        return (false);
    } 
    if (_scriptFileName.find("../") != std::string::npos){
        // std::cout << G"I fail!!!! 4" << _scriptFileName;
        // std::cout << B"\n";
        return (false);
    } 
    return (true);
}

// int CGI::getFd() { return (_fd);}