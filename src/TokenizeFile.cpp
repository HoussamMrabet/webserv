/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TokenizeFile.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 09:47:04 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/21 10:54:31 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TokenizeFile.hpp"

TokenizeFile::TokenizeFile(void) {
    return ;
}

TokenizeFile::TokenizeFile(const TokenizeFile &copy) {
    *this = copy; 
}

TokenizeFile &TokenizeFile::operator=(const TokenizeFile &copy) {
    if (this != &copy)
        *this = copy;
    return (*this);
}

TokenizeFile::~TokenizeFile(void) {
    return ;
}

const char *TokenizeFile::CantOpenFileException::what() const throw() {
    return "Can't open the config file !";
}

std::string TokenizeFile::openFile(std::string file) {
    std::string line;
    std::string buffFile;
    std::ifstream inFile(file.c_str());

    if (!inFile) {
        throw TokenizeFile::CantOpenFileException();
    }
    while (std::getline(inFile, line)) {
        size_t found = line.find("#");
        if (found != std::string::npos) {
            line = line.substr(0, found); 
        }
        if (line.empty()) continue;  
        line = addSpaces(line);  
        size_t pos = 0;
        while ((found = line.find('\t', pos)) != std::string::npos) {
            buffFile += line.substr(pos, found - pos) + " ";
            pos = found + 1;
        }
        if (pos < line.length())  
            buffFile += line.substr(pos) + " ";
    }
    inFile.close(); 
    return buffFile;
}

std::string addSpaces(std::string str) {
    std::string res;

    if (str.empty())
        return res;

    for (size_t i = 0; i < str.size(); i++) {
        char c = str[i];

        if (c == '{' || c == '}' || c == ';') {
            if (!res.empty() && res[res.size() - 1] != ' ') {
                res += ' ';
            }
            res += c;
            if ((i + 1) < str.size() && str[i + 1] != ' ') {
                res += ' ';
            }
        } else {
            res += c;
        }
    }
    return res;
}

std::vector<std::string> TokenizeFile::tokens(std::string file) {
    std::vector<std::string> res;
    std::string buffer = TokenizeFile::openFile(file);
    std::istringstream stream(buffer);
    std::string word;

    while (stream >> word) {
        res.push_back(word);
    }

    return (res);
}

