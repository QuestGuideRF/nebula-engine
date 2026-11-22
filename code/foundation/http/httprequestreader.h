#pragma once
#ifndef HTTP_HTTPREQUESTREADER_H
#define HTTP_HTTPREQUESTREADER_H
#include "io/streamreader.h"
#include "http/httpmethod.h"
#include "io/uri.h"
namespace Http
{
class HttpRequestReader : public IO::StreamReader
{
    __DeclareClass(HttpRequestReader);
public:
    HttpRequestReader();
    bool ReadRequest();
    bool IsValidHttpRequest() const;
    HttpMethod::Code GetHttpMethod() const;
    const IO::URI& GetRequestURI() const;
private:
    bool isValidHttpRequest;
    HttpMethod::Code httpMethod;
    IO::URI requestURI;
};
inline bool
HttpRequestReader::IsValidHttpRequest() const
{
    return this->isValidHttpRequest;
}
inline HttpMethod::Code
HttpRequestReader::GetHttpMethod() const
{
    return this->httpMethod;
}
inline const IO::URI&
HttpRequestReader::GetRequestURI() const
{
    return this->requestURI;
}
}
#endif