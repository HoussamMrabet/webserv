#pragma once

#include <iostream>
#include <map>

class MimeTypes
{
    private :
        std::map<std::string, std::string>   mime;
    public:
        MimeTypes();
        ~MimeTypes();
        std::string getMimeType(std::string ext);
};