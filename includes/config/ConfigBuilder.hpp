/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 21:06:21 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/04/18 07:18:50 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <vector>
# include <map>
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
};