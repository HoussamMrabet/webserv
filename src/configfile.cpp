/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configfile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 06:24:37 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/10 07:55:10 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>

void openFile(std::string file) {

	std::string line;
	std::string buffFile;
	std::ifstream inFile(file.c_str());
	size_t found;
	
	if (!inFile)
	{
		std::cout << "Cant open file : " << file << std::endl;
		return ;
	}
	 while (std::getline(inFile, line))
	 {
		if ((found = line.find("#", 0)) != std::string::npos )
		{
			line = line.substr(0, found);
		}
		size_t pos;
		
		pos = 0;
		while ((found = line.find('\t', pos)) != std::string::npos) {
			buffFile += line.substr(pos, found - pos);
			buffFile += " ";
			pos = found + 1;
		}
		buffFile += line.substr(pos) + " ";
	 }
	 std::cout << buffFile << std::endl;
}