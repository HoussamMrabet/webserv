#include "Request.hpp"

void Request::setBodyInformations()
{
    std::map<std::string, std::string>::iterator transferEncodingIt = this->headers.find("transfer-encoding");
    if (transferEncodingIt != this->headers.end())
    {
        std::string transferEncoding = transferEncodingIt->second;
        std::transform(transferEncoding.begin(), transferEncoding.end(), transferEncoding.begin(), ::tolower);
        if (transferEncoding != "chunked")
        {
            this->message = "Unsupported Transfer-Encoding";
            throw 501;
        }
        this->isChunked = true;
    }

    std::map<std::string, std::string>::iterator contentLengthIt = this->headers.find("content-length");
    if (contentLengthIt != this->headers.end())
    {
        char *end;
        this->contentLength = strtol(contentLengthIt->second.c_str(), &end, 10);
        if ((signed)contentLength < 0) ////////!!!!
        {
            this->message = "Invalid Content-Length";
            throw 400;
        }
        this->isContentLength = true;
    }

    std::map<std::string, std::string>::iterator contentTypeIt = this->headers.find("content-type");
    if (contentTypeIt != this->headers.end())
    {
        std::string contentType = contentTypeIt->second;
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos)
        {
            this->boundaryKey = "--" + contentType.substr(boundaryPos + 9);
            this->isMultipart = true;
            this->boundaryKey.erase(this->boundaryKey.size());
        }
    }
    if (!this->isMultipart)
    {
        this->file.open(generateRandomFileName(this->uploadDir + "/binary_file_").c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        if (!this->file.is_open())
        {
            this->message = "Failed to open file";
            throw 500;
        }
    }
}

void Request::parseRequest(const std::string &rawRequest)
{    
    if (this->statusCode != 200 || this->currentStep == DONE)
        return ;
    try
    {
        this->requestData += rawRequest;
        if (this->currentStep == REQ_LINE)
        {
            size_t pos = this->requestData.find("\r\n");
            if (pos != std::string::npos)
            {
                this->reqLine = this->requestData.substr(0, pos);
                this->requestData.erase(0, pos + 2);
                this->parseRequestLine();
                this->currentStep = HEADERS;
            }
        }
        if (this->currentStep == HEADERS)
        {
            size_t pos = this->requestData.find("\r\n\r\n");
            if (pos != std::string::npos)
            {
                this->headersData = this->requestData.substr(0, pos);
                this->requestData.erase(0, pos + 4);
                this->parseHeaders();
                this->processResponseErrors();
                this->setBodyInformations();
                this->currentStep = BODY;
            }
        }

        if (this->currentStep == BODY)
        {
            this->fullBody += this->requestData;
            this->body += this->requestData;
            this->currentContentLength += requestData.size();
            this->requestData.clear();
            this->parseBody();
        }
    }
    catch(const int &e)
    {
        this->statusCode = e;
        this->currentStep = DONE;
    }
}
