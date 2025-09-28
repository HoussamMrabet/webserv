/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:16:26 by hmrabet           #+#    #+#             */
/*   Updated: 2025/02/17 15:41:17 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include "MimeTypes.hpp"
#define CRLF "\r\n"
#define BUFFER_SIZE 4096


class Response {
	private :
	std::string status;
	std::string status_message;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::string body;
	size_t content_length;
	bool is_chunked;
	bool connection_close;
	bool is_ready;
	bool has_body;

	// For chunked file sending
	std::string file_path;
	std::ifstream file_stream;
	size_t bytes_sent;
	bool headers_sent;
	std::string cached_headers;

	void init_status_map();
	static std::map<int, std::string> status_map;
	
	public :
		// Constructor and basic setup
		Response();
		Response(int status_code);
		~Response();
		
		// Static method for simple error responses (keep existing)
		static std::string getResponse(int code);
		
		// New methods for building responses
		void setStatus(int status_code);
		void setHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& content);
		void setBodyFromFile(const std::string& file_path);
		void setHttpVersion(const std::string& version); // New method to set HTTP version
		
		// Response generation methods
		std::string buildResponse();
		std::string buildGetResponse(const std::string& file_path);
		std::string buildPostResponse();
		std::string buildDeleteResponse(const std::string& file_path);
		
		// Chunked response methods for large files
		void prepareFileResponse(const std::string& file_path);
		std::string getNextChunk(size_t chunk_size = BUFFER_SIZE);
		bool isResponseComplete() const;
		void reset();
		
		// Universal response method - works for any file size
		void prepareResponse(const std::string& file_path);
		std::string getResponseChunk(size_t chunk_size = BUFFER_SIZE);
		bool isFinished() const;
		
		// Utility methods
		std::string getContentType(const std::string& file_path);
		bool fileExists(const std::string& file_path);
		std::string getStatusMessage(int code);
		size_t getFileSize(const std::string& file_path);

		void parseCgiOutput(const std::string &cgi_output);

};