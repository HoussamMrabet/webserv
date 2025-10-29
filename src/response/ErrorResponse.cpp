/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorResponse.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 18:05:05 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/10/29 18:05:20 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Connection.hpp"

void Connection::sendErrorPage(Request &request, int code, ServerConf &server){
    Response response_obj;
    std::string error_page;
    std::map<std::string, std::string> error_pages = server.getErrorPages();
    std::string connection_header = getConnectionHeader(request);
    
    if (error_pages.find(to_str(code)) != error_pages.end()) {
        error_page = error_pages[to_str(code)];
        std::string full_error_path = error_page;
        if (response_obj.fileExists(full_error_path)) {
            response_obj.setStatus(code);
            response_obj.setBodyFromFile(full_error_path);
            response_obj.setHeader("Content-Type", response_obj.getContentType(full_error_path));
            response_obj.setHeader("Connection", connection_header);
            _response = response_obj.buildResponse();
            _isChunkedResponse = false;
            return;
        }
    }
    
    std::string standard_error_page = request.getRoot() + "/errors/" + to_str(code) + ".html";
    if (response_obj.fileExists(standard_error_page)) {
        response_obj.setStatus(code);
        response_obj.setBodyFromFile(standard_error_page);
        response_obj.setHeader("Content-Type", response_obj.getContentType(standard_error_page));
        response_obj.setHeader("Connection", connection_header);
        _response = response_obj.buildResponse();
        _isChunkedResponse = false;
        return;
    }
    response_obj.setStatus(code);
    response_obj.setBody("<html><body><h1>" + to_str(code) + " " + response_obj.getStatusMessage(code) + "</h1></body></html>");
    response_obj.setHeader("Content-Type", "text/html");
    response_obj.setHeader("Connection", connection_header);
    _response = response_obj.buildResponse();
    _isChunkedResponse = false;
}