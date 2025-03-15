/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request-utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 06:38:26 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/15 07:14:27 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void handleUriSpecialCharacters(std::string &uri)
{
    std::string encodedChars[] = {"%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27", "%28", "%29", "%2A", "%2B", "%2C",
                                  "%2D", "%2E", "%2F", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F", "%40", "%5B", "%5C", "%5D",
                                  "%5E", "%5F", "%60", "%7B", "%7C", "%7D", "%7E"};

    std::string specialChars[] = {" ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", ":", ";", "<",
                                  "=", ">", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "|", "}", "~"};

    for (size_t i = 0; i < sizeof(encodedChars) / sizeof(encodedChars[0]); i++)
    {
        std::string::size_type n = 0;
        while ((n = uri.find(encodedChars[i], n)) != std::string::npos)
        {
            uri.replace(n, encodedChars[i].length(), specialChars[i]);
            n += 1;
        }
    }
}

std::string generateRandomFileName(const std::string &prefix)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    int len = 10;
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

    std::ifstream file(prefix + tmp_s);
    if (file.good())
        return (generateRandomFileName());
    return (prefix + tmp_s);
}
