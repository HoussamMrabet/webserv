/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configfile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: voop <voop@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 06:24:37 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/20 11:15:02 by voop             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>


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

std::string openFile(std::string file) {
    std::string line;
    std::string buffFile;
    std::ifstream inFile(file.c_str());

    if (!inFile) {
        std::cerr << "Cant open file: " << file << std::endl;
        return "";
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

    std::cout << "Final Processed File Content: " << buffFile << std::endl;

    return buffFile;
}

