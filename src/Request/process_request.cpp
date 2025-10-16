#include "Request.hpp"
#include "WebServer.hpp"
#include <sys/stat.h> // move to header

void Request::processRequest(){
    const ServerConf &server = globalServer[0];
    std::map<std::string, LocationConf> locations = server.getLocations();
    std::string document_root;
    std::string full_path;
    std::string index;
    struct stat fileStat;
    
    // Find the matching location configuration
    std::string location_path = "/";
    
    // Find the best matching location (longest prefix match)
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); 
        it != locations.end(); ++it) {
        if (this->uri.find(it->first) == 0 && it->first.length() > location_path.length()) {
            location_path = it->first;
        }
    } // what if location is not found!!
    if (locations.find(location_path) == locations.end()) {
        // No matching location found, use default root
        document_root = server.getRoot();
        if (document_root[0] != '.') // make sure document_root starts with .
            document_root = "." + document_root;
        full_path = document_root + this->uri;
    }
    else{
        document_root = locations[location_path].getRoot();
        if (document_root.empty())
            document_root = server.getRoot();
        // std::cout << "*-*-*-*-*-*-*-> PB here 3 <-*-*-*-*-*-*\n";
        std::string file_name = this->uri.substr(std::min(location_path.length(), this->uri.length()));
        if (file_name.length() > 0 && file_name[0] == '/')
            file_name = file_name.substr(1);
        // std::cout << "*-*-*-*-*-*-*-> file_name: " << file_name << std::endl;
        if (document_root[0] != '.') // make sure document_root starts with .
            document_root = "." + document_root;
        full_path = document_root + "/" + file_name;
        if (file_name.empty()) {
            // std::cout << "*-*-*-*-*-*-*-> EMPTY file_name <-*-*-*-*-*-*\n";
            // bool auto_index = locations.at(location_path).getAutoIndex();
            // if (auto_index){
            std::vector<std::string> indexes = locations[location_path].getIndex();
            if (indexes.empty()) {
                indexes = server.getIndex();
            }
            for (std::vector<std::string>::const_iterator it = indexes.begin(); it != indexes.end(); ++it) {
                std::string index_path = document_root;
                if (index_path[index_path.length() - 1] != '/') {
                    index_path += "/";
                }
                index_path += *it;
                index = *it;
                // std::cout << "*-*-*-*-*-*-*-> index_path: " << index_path << std::endl;
                if (stat(index_path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                    full_path = index_path;
                    break;
                }
            }

            // }
            // else should i check for directory listing
            // or return error page?
        }
        // std::cout << "*-*-*-*-*-*-*-> full_path: " << full_path << std::endl;

    }

    this->fullPath = full_path;
    // std::cout << "*-*-*-*-*-*-*-> full_path: " << full_path << std::endl;
    // std::cout << "*-*-*-*-*-*-*-> index: " << index << std::endl;
    // std::cout << "*-*-*-*-*-*-*-> PB here 4 <-*-*-*-*-*-*\n";

    // /**************************************************/
    if (this->fullPath.length() >= 4 && this->fullPath.substr(this->fullPath.length() - 4) == ".php"){
        this->cgiType = ".php";
        this->uriFileName = index;

    }
    else if (this->fullPath.length() >= 3 && this->fullPath.substr(this->fullPath.length() - 3) == ".py"){
        this->cgiType = ".py";
        this->uriFileName = index;
    }
    // if (this->isCGI())
    // {
    //     std::string filename = generateRandomFileName("./");
    //     this->createdFile = filename;
    //     int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
    //     if (fd == -1)
    //     {
    //         this->message = "Failed to open temporary file";
    //         throw 500;
    //     }

    //     if (remove(filename.c_str()) != 0)
    //     {
    //         close(fd);
    //         this->message = "Failed to remove temporary file";
    //         throw 500;
    //     }

    //     int fd2 = dup(fd);
    //     if (fd2 == -1)
    //     {
    //         close(fd);
    //         this->message = "Failed to duplicate file descriptor";
    //         throw 500;
    //     }

    //     std::cout << "-*-*-*-*-*-*- >>>> pb here!!!!!!!!" << this->message << "\n";
    //     this->cgiFdRead = fd;
    //     this->cgiFdWrite = fd2;
    // }

    int fd = open(fullPath.c_str(), O_RDONLY);
    if (fd == -1){
        // std::cout << G"-------- Im the problem 1!!!!! ";
        // std::cout << B"\n";
        this->statusCode = 404;
        return;
    }
    else
        close(fd);
    // if (access(fullPath.c_str(), F_OK) != 0){

    //     // std::cout << G"I fail!!!! 1" << fullPath;
    //     // std::cout << B"\n";
    //     this->statusCode = 404;
    // } 
    // if (access(fullPath.c_str(), R_OK) != 0){
    //     // std::cout << G"I fail!!!! 2" << fullPath;
    //     // std::cout << B"\n";
    //     this->statusCode = 404;
    // } 
    // if (access(fullPath.c_str(), X_OK) != 0){
    //     // std::cout << G"I fail!!!! 3" << _execPath;
    //     // std::cout << G"I fail!!!! 3" << fullPath;
    //     // std::cout << B"\n";
    //     this->statusCode = 404;
    // } 
    // if (fullPath.find("../") != std::string::npos){
    //     // std::cout << G"I fail!!!! 4" << _scriptFileName;
    //     // std::cout << B"\n";
    //     this->statusCode = 404;
    // } 
}

std::string Request::getFullPath() const{
    return (this->fullPath);
}