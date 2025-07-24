
#include "Request.hpp"

void Request::parseRequestLine()
{
    if (!this->reqLine.empty() && (this->reqLine[0] == ' ' || this->reqLine[0] == '\t'))
    {
        this->message = "Invalid request line";
        throw 400;
    }

    std::istringstream lineStream(this->reqLine);
    std::string methodStr;

    if (!(lineStream >> methodStr >> this->uri >> this->httpVersion))
    {
        this->message = "Invalid request line";
        throw 400;
    }

    if (methodStr == "GET")
        this->method = GET;
    else if (methodStr == "POST")
        this->method = POST;
    else if (methodStr == "DELETE")
        this->method = DELETE;
    else
    {
        this->message = "Invalid Method";
        throw 400;
    }
    
    handleUriSpecialCharacters(this->uri);
    if (this->uri[0] != '/')
    {
        this->message = "Invalid URI";
        throw 400;
    }
    
    if (httpVersion.size() < 6 || httpVersion.substr(0, 5) != "HTTP/")
    {
        this->message = "Invalid request line";
        throw 400;
    }

    std::string version = httpVersion.substr(5);

    if (version.empty() || version.find_first_not_of("0123456789.") != std::string::npos)
    {
        this->message = "Invalid request line";
        throw 400;
    }
    
    double versionNumber = atof(version.c_str());
    if (versionNumber < 1.0)
    {
        this->message = "Invalid request line";
        throw 400;
    }

    if (httpVersion != "HTTP/1.1")
    {
        this->message = "HTTP Version Not Supported";
        throw 505;
    }

    std::string extra;
    if (lineStream >> extra)
    {
        this->message = "Invalid request line";
        throw 400;
    }
}
