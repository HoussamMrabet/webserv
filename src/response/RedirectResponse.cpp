/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectResponse.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 18:07:45 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/10/29 18:07:46 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Connection.hpp"

std::string Connection::checkForRedirect(Request &request, ServerConf &server) {
    std::string requested_path = request.getUri();
    std::map<std::string, LocationConf> locations = server.getLocations();
    
    std::string best_match = "";
    const LocationConf* best_location = NULL;
    if ((requested_path == "/profile/profile.html" ) && Request::loggedIn == false)
        return "/profile/login.html";
    if ((requested_path == "/profile" || requested_path == "/profile/"||requested_path == "/profile/login.html") && Request::loggedIn == true)
        return "/profile/profile.html";
    if (requested_path == "/logout")
        return "/profile";
    for (std::map<std::string, LocationConf>::const_iterator it = locations.begin(); 
         it != locations.end(); ++it) {
        if (requested_path.find(it->first) == 0 && it->first.length() > best_match.length()) {
            best_match = it->first;
            best_location = &(it->second);
        }
    }
    
    if (best_location != NULL) {
        std::string redirect_url = best_location->getRedirectUrl();
        if (!redirect_url.empty()) {
            return redirect_url;
        }
    }
    
    return ""; 
}

std::string Connection::sendRedirectResponse(Request &request, const std::string &redirect_url, ServerConf &server) {
    Response response_obj;
    std::string connection_header = getConnectionHeader(request);
    (void)server; 
    

    response_obj.setStatus(301);
    response_obj.setHeader("Location", redirect_url);
    response_obj.setHeader("Connection", connection_header);
    response_obj.setHeader("Content-Type", "text/html");
    
    std::string redirect_body = "<html><body><h1>301 Moved Permanently</h1>"
                               "<p>The document has moved <a href=\"" + redirect_url + "\">here</a>.</p>"
                               "</body></html>";
    
    response_obj.setBody(redirect_body);
    std::string response = response_obj.buildResponse();
    _isChunkedResponse = false;
    return response;
}