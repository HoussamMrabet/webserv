/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:12:37 by hmrabet           #+#    #+#             */
/*   Updated: 2025/01/19 15:50:49 by hmrabet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <cstring> // for npos
// # define DEFAUl_PATH // if no arg

// on linux:
// make
// run: ./webserv
// in an other terminal run: python client.py
// note: client.py is used only for testing

int main(int ac, char **av)
{
    // if (ac == 1 
    if (ac == 2){
        // parse config file name
        // parse config file
        // creat server
        std::string s = av[1];
        if (s.size() < 7 || s.substr(s.size() - 7, s.size() - 1) != ".config") std::cout << "NO" << std::endl;
        else std::cout << "OK" << std::endl;
        // Server server("0.0.0.0", 1337);
    }


    // std::cout << "webserv" << std::endl;
    // Server server("0.0.0.0", 1337);
}