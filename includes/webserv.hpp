/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:13:55 by hmrabet           #+#    #+#             */
/*   Updated: 2025/04/18 07:08:37 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
// #include "server.hpp"
// #include "request.hpp"
// #include "response.hpp"
#include "cgi.hpp"
#include "./config/ConfigBuilder.hpp"
#include "TokenizeFile.hpp"


std::string openFile(std::string file);