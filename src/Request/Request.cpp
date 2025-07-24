
#include "Request.hpp"

Request::Request() : statusCode(200), message("success!"), currentStep(REQ_LINE),
                        method(UNDEFINED), reqLine(""), uri(""), httpVersion(""),
                        body(""), isChunked(false), isMultipart(false),
                        isContentLength(false), boundaryKey(""), contentLength(0),
                        requestData(""), headersData(""), currentContentLength(0),
                        fileName(""), fullBody(""), chunkSize(0), chunkData(""), inChunk(false)
{
    this->headers["connection"] = "keep-alive";
}
Request::~Request()
{
    for (size_t i = 0; i < multipartData.size(); ++i)
        delete multipartData[i];
    if (file.is_open())
        file.close();
}

t_method Request::getMethod() const
{
    return (this->method);
}

int Request::getStatusCode() const
{
    return (this->statusCode);
}

std::string Request::getUri() const
{
    return (this->uri);
}

std::map<std::string, std::string> Request::getHeaders() const
{
    return (this->headers);
}

std::string Request::getBody() const
{
    return (this->fullBody);
}

std::string Request::getHeader(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = this->headers.find(key);
    if (it != this->headers.end())
    return it->second;
    return "";
}

std::string Request::getHost() const
{
    return (getHeader("host"));
}

bool Request::isDone() const
{
    return (this->currentStep == DONE);
}

void Request::printRequest()
{
    if (!isDone())
        return ;
    std::cout << this->reqLine << std::endl
              << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); it++)
        std::cout << it->first << ": " << it->second << std::endl;
    std::cout << std::endl;
    if (!this->fullBody.empty())
        std::cout << this->fullBody << std::endl;
}
