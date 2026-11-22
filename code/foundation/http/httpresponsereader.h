#pragma once
#include "io/streamreader.h"
#include "http/httpstatus.h"
#include "io/mediatype.h"
namespace Http
{
class HttpResponseReader : public IO::StreamReader
{
    __DeclareClass(HttpResponseReader);
public:
    HttpResponseReader();
    bool ReadResponse();
    bool IsValidHttpResponse() const;
    HttpStatus::Code GetStatusCode() const;
    const IO::MediaType& GetContentType() const;
    SizeT GetContentLength() const;
private:
    bool isValidHttpResponse;
    HttpStatus::Code httpStatus;
    IO::MediaType contentType;
    SizeT contentLength;
};
inline bool
HttpResponseReader::IsValidHttpResponse() const
{
    return this->isValidHttpResponse;
}
inline HttpStatus::Code
HttpResponseReader::GetStatusCode() const
{
    return this->httpStatus;
}
inline const IO::MediaType&
HttpResponseReader::GetContentType() const
{
    return this->contentType;
}
inline SizeT
HttpResponseReader::GetContentLength() const
{
    return this->contentLength;
}
}