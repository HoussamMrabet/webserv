/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 21:06:21 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/22 11:52:25 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iomanip>
# include <exception>
# include <sstream>
# include <iostream>
# include <vector>
# include <set>
# include <map>
# include <stack>
# include <utility>
# include "TokenizeFile.hpp"
#include "LocationConf.hpp"
#include "ServerConf.hpp"
# include "LocationConf.hpp"
#include <cstdlib>


class ServerConf;

class ConfigBuilder {

	private :
		static ServerConf _server;
	
	public :
		ConfigBuilder();
		ConfigBuilder(const ConfigBuilder &copy);
		ConfigBuilder &operator = (const ConfigBuilder &copy);
		virtual ~ConfigBuilder();

		static std::vector<ServerConf> generateServers(std::string file);
		static ServerConf buildServer(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens, std::stack<char> &stk);
		static bool checkDirective(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens);
		static bool checkPort(std::string str);
		static bool checkIp(std::string& str);
		static ServerConf getServer();
		class ErrorConfig : public std::exception {
			private :
				const std::string msg;
			public :
				ErrorConfig(std::string msg) : msg(msg) {

				}

				const char* what() const throw() {
					return (this->msg.c_str());
				}

				~ErrorConfig() throw() {
					
				}
		};

};