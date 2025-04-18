/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 21:06:21 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 15:10:10 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <vector>
# include <map>
# include <stack>
# include <utility>
# include "LocationConf.hpp"
# include "ServerConf.hpp"
# include "TokenizeFile.hpp"


class ConfigBuilder {

	private :
	
	public :
		ConfigBuilder();
		ConfigBuilder(const ConfigBuilder &copy);
		ConfigBuilder &operator = (const ConfigBuilder &copy);
		virtual ~ConfigBuilder();

		static std::vector<ServerConf> generateServers(std::string file);
		static ServerConf buildServer(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		static bool checkDirective(std::vector<std::string>::const_iterator &it,  std::vector<std::string> &tokens);
};