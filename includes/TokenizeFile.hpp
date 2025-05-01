/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TokenizeFile.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 09:43:35 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/21 10:44:52 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <vector>
# include <fstream>
# include <exception>
# include <sstream>

class TokenizeFile {
    private :
    
    public :
        TokenizeFile(void);
        TokenizeFile(const TokenizeFile &copy);
        TokenizeFile &operator=(const TokenizeFile &copy);
        virtual ~TokenizeFile(void);
    
        class CantOpenFileException: public std::exception {
            virtual const char *what() const throw();
        };
    
        static std::string openFile(std::string file);
        static std::vector<std::string> tokens(std::string file);
};

std::string addSpaces(std::string str);