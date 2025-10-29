#include "Request.hpp"
#include "WebServ.hpp"

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
        if (contentLength < 0)
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
            if (this->boundaryKey.size() > 70)
            {
                this->message = "Invalid boundary length";
                throw 400;
            }
        }
    }
    if (this->getMethod() != POST)
        throw 200;
    if (!this->isMultipart && !this->isCGI())
    {
        if (this->headers.find("content-type") != this->headers.end())
        {
            std::string contentType = this->headers["content-type"];

            try
            {
                checkMediaType(contentType);
            }
            catch (const char *error)
            {
                this->message = error;
                throw 415;
            }

            if (contentType.find("application/x-www-form-urlencoded") != std::string::npos ||
                contentType.find("application/json") != std::string::npos ||
                contentType.find("text/") != std::string::npos)
            {
                return;
            }
        }
        else
        {
            return;
        }
        std::string filename = generateRandomFileName(this->uploadDir + "/binary_file_");
        this->createdFile = filename;
        int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd == -1)
        {
            this->message = "Failed to open file";
            throw 500;
        }

        this->file = fd;
    }
}

void Request::parseRequest(const std::string &rawRequest)
{
    if (this->statusCode != 200 || this->currentStep == DONE)
        return;
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
            else if (this->requestData.size() > 8192)
            {
                this->message = "Request Line Too Long";
                throw 414;
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
                this->handleThemeCookie();
                this->handleSession();
                if (!this->isCGI())
                    this->processResponseErrors();
                this->setBodyInformations();
                this->currentStep = BODY;
            }
            else if (this->requestData.size() > 8192)
            {
                this->message = "Request Headers Too Long";
                throw 431;
            }
        }

        if (this->currentStep == BODY)
        {
            if (this->getMethod() != POST)
                throw 200;

            this->fullBody += this->requestData;

            std::map<std::string, LocationConf> locations = server.getLocations();
            std::map<std::string, LocationConf>::iterator locIt = locations.find(this->location);

            if (locIt != locations.end())
            {
                const LocationConf &location = locIt->second;
                if (this->fullBody.size() > location.getBodySizeLimit())
                {
                    if (!this->createdFile.empty())
                        remove(this->createdFile.c_str());
                    if (this->isMultipart && !this->multipartData.empty())
                        this->multipartData.back()->unlinkFile();
                    this->message = "Request Body Too Large";
                    throw 413;
                }
            }
            else if (this->fullBody.size() > this->contentLength || this->fullBody.size() > server.getBodySizeLimit())
            {
                this->message = "Request Body Too Large";
                throw 413;
            }
            this->body += this->requestData;
            this->currentContentLength += requestData.size();
            this->requestData.clear();
            this->parseBody();
        }
        if (this->lastChunk)
            while (true)
                parseBody();
    }
    catch (const int &e)
    {
        this->statusCode = e;
        this->currentStep = DONE;
        if (this->statusCode != 200)
            close(this->cgiFdRead);
        close(this->cgiFdWrite);
    }
}
