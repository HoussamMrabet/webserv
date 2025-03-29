/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 21:06:21 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/03/29 13:22:52 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <vector>
# include <map>
# include <utility>
# include "LocationConf.hpp"
# include "ServerConf.hpp"


class ConfBuilder {

	private :
	
	public :
		ConfBuilder();
		ConfBuilder(const ConfBuilder &copy);
		ConfBuilder &operator = (const ConfBuilder &copy);
		virtual ~ConfBuilder();

		static std::vector<ServerConf> generateServers(std::string file);
};