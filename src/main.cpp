/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:12:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/03/24 10:55:20 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <iostream>

int main()
{
    try
    {
        std::vector<std::string> tokens = TokenizeFile::tokens("config/sdefault.conf");
    
        for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
            std::cout << *it << std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    
    
    
    return (0);
}