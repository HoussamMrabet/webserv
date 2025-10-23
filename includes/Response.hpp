#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "ServerConf.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <ctime>

class Response {
	private:
		int _statusCode;
		std::string _statusMessage;
		std::string _body;
		std::map<std::string, std::string> _headers;

	public:
		Response();
		Response(int code, const std::string& message);
		~Response();
		void setStatus(int code, const std::string& message);
		void setHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);
		std::string getStatusMessage(int code) const;
		void handle(const Request& req, const ServerConf& config);
		void handleGET(const Request& req, const ServerConf& config);
		void handlePOST(const Request& req, const ServerConf& config);
		void handleDELETE(const Request& req, const ServerConf& config);
		void sendError(int code);
		void sendFile(const std::string& path);
		void sendDirectory(const std::string& path, const std::string& uri, bool autoindex);
		std::string readFile(const std::string& path) const;
		std::string getContentType(const std::string& path) const;
		std::string generateDirectoryListing(const std::string& path, const std::string& uri) const;
		void fromCGI(const std::string& cgiOutput);
		std::string build() const;

};

#endif