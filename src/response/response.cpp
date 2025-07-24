/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 15:34:45 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/02/17 15:34:46 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::string Response::getResponse(int code){
    std::string response;
    switch (code)
    {
        case 200:
            response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nDone!";
            break;
        case 400:
            response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 11\r\n\r\nBad Request";
            break;
        case 500:
            response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 21\r\n\r\nInternal Server Error";
            break;
        case 501:
            response = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 17\r\n\r\nNot Implemented";
            break;
        case 505:
            response = "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 29\r\n\r\nHTTP Version Not Supported";
            break;
        case 413:
            response = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 20\r\n\r\nPayload Too Large";
            break;
        case 415:
            response = "HTTP/1.1 415 Unsupported Media Type\r\nContent-Length: 26\r\n\r\nUnsupported Media Type";
            break;
        default:
            response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 21\r\n\r\nInternal Server Error";
            break;
    }
    return (response);
}