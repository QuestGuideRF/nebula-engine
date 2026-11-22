#pragma once
#ifndef HTTP_HTTPRESPONSEWRITER_H
#define HTTP_HTTPRESPONSEWRITER_H
#include "io/streamwriter.h"
#include "http/httpstatus.h"
namespace Http
{
class HttpResponseWriter : public IO::StreamWriter
{
    __DeclareClass(HttpResponseWriter);
public:
    void SetStatusCode(HttpStatus::Code statusCode);
    void SetContent(const Ptr<IO::Stream>& contentStream);
    void WriteResponse();
private:
    HttpStatus::Code statusCode;
    Ptr<IO::Stream> contentStream;
};
inline void
HttpResponseWriter::SetStatusCode(HttpStatus::Code c)
{
    this->statusCode = c;
}
inline void
HttpResponseWriter::SetContent(const Ptr<IO::Stream>& s)
{
    this->contentStream = s;
}
}
#endif