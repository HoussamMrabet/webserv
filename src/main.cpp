/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:12:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/04/18 07:19:38 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main()
{
    // try
    // {
    //     std::vector<std::string> tokens = TokenizeFile::tokens("config/default.conf");
    
    //     for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
    //         std::cout << *it << std::endl;
    //     }
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
    
    ConfigBuilder::generateServers("config/default.conf");
    
    
    return (0);
}