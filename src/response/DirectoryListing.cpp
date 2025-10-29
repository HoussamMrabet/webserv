/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryListing.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 18:04:25 by mel-hamd          #+#    #+#             */
/*   Updated: 2025/10/29 18:04:59 by mel-hamd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Connection.hpp"

std::string Connection::generateDirectoryListing(const std::string& directory_path, const std::string& request_uri) {
    std::stringstream html;
    
    html << "<!DOCTYPE html>\n<html>\n<head>\n";
    html << "<title>Index of " << request_uri << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "h1 { border-bottom: 1px solid #ccc; }\n";
    html << "table { border-collapse: collapse; width: 100%; }\n";
    html << "th, td { text-align: left; padding: 8px 12px; border-bottom: 1px solid #ddd; }\n";
    html << "th { background-color: #f2f2f2; }\n";
    html << "tr:hover { background-color: #f5f5f5; }\n";
    html << "a { text-decoration: none; color: #0066cc; }\n";
    html << "a:hover { text-decoration: underline; }\n";
    html << ".dir { font-weight: bold; }\n";
    html << "</style>\n</head>\n<body>\n";
    html << "<h1>Index of " << request_uri << "</h1>\n";
    
    DIR* dir = opendir(directory_path.c_str());
    if (dir == NULL) {
        html << "<p>Error: Cannot read directory</p>\n";
        html << "</body>\n</html>";
        return html.str();
    }
    
    std::vector<std::pair<std::string, bool> > entries; 
    struct dirent* entry;
    struct stat entry_stat;
    
    while ((entry = readdir(dir)) != NULL) {
        std::string entry_name = entry->d_name;
        
        if (entry_name[0] == '.' && entry_name != "..") {
            continue;
        }
        
        std::string full_entry_path = directory_path + "/" + entry_name;
        bool is_directory = false;
        
        if (stat(full_entry_path.c_str(), &entry_stat) == 0) {
            is_directory = S_ISDIR(entry_stat.st_mode);
        }
        
        entries.push_back(std::make_pair(entry_name, is_directory));
    }
    closedir(dir);
    
    for (size_t i = 0; i < entries.size(); i++) {
        for (size_t j = i + 1; j < entries.size(); j++) {
            bool should_swap = false;
            
            if (entries[i].second != entries[j].second) {
                should_swap = entries[j].second;
            } else {
                should_swap = entries[i].first > entries[j].first;
            }
            
            if (should_swap) {
                std::pair<std::string, bool> temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }
    html << "<table>\n";
    html << "<tr><th>Name</th><th>Type</th><th>Size</th></tr>\n";
    
    for (std::vector<std::pair<std::string, bool> >::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        const std::string& entry_name = it->first;
        bool is_directory = it->second;
        
        html << "<tr>";
        std::string href = "";
        if (href[href.length() - 1] != '/') {
            href += "/";
        }
        href += entry_name;        
        html << "<td><a href=\"" << href << "\"";
        if (is_directory) {
            html << " class=\"dir\"";
        }
        html << ">" << entry_name;
        if (is_directory) {
            html << "/";
        }
        html << "</a></td>";        
        html << "<td>" << (is_directory ? "Directory" : "File") << "</td>";
        std::string full_entry_path = directory_path + "/" + entry_name;
        struct stat entry_stat;
        if (stat(full_entry_path.c_str(), &entry_stat) == 0 && !is_directory) {
            html << "<td>" << entry_stat.st_size << " bytes</td>";
        } else {
            html << "<td>-</td>";
        }
        
        html << "</tr>\n";
    }
    
    html << "</table>\n";
    html << "</body>\n</html>";
    
    return html.str();
}