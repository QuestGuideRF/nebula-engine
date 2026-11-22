#pragma once
#ifndef HTTP_HTTPREQUEST_H
#define HTTP_HTTPREQUEST_H
#include "messaging/message.h"
#include "http/httpmethod.h"
#include "http/httpstatus.h"
#include "io/uri.h"
namespace Http
{
class HttpRequest : public Messaging::Message
{
    __DeclareClass(HttpRequest);
    __DeclareMsgId;
public:
    HttpRequest();
    virtual ~HttpRequest();
    void SetMethod(HttpMethod::Code requestMethod);
    HttpMethod::Code GetMethod() const;
    void SetURI(const IO::URI& requestUri);
    const IO::URI& GetURI() const;
    void SetResponseContentStream(const Ptr<IO::Stream>& responseContentStream);
    const Ptr<IO::Stream>& GetResponseContentStream() const;
    void SetStatus(HttpStatus::Code status);
    HttpStatus::Code GetStatus() const;
private:
    HttpMethod::Code method;
    IO::URI uri;
    Ptr<IO::Stream> responseContentStream;
    HttpStatus::Code status;
};
inline void
HttpRequest::SetMethod(HttpMethod::Code m)
{
    this->method = m;
}
inline HttpMethod::Code
HttpRequest::GetMethod() const
{
    return this->method;
}
inline void
HttpRequest::SetURI(const IO::URI& u)
{
    this->uri = u;
}
inline const IO::URI&
HttpRequest::GetURI() const
{
    return this->uri;
}
inline void
HttpRequest::SetResponseContentStream(const Ptr<IO::Stream>& s)
{
    this->responseContentStream = s;
}
inline const Ptr<IO::Stream>&
HttpRequest::GetResponseContentStream() const
{
    return this->responseContentStream;
}
inline void
HttpRequest::SetStatus(HttpStatus::Code s)
{
    this->status = s;
}
inline HttpStatus::Code
HttpRequest::GetStatus() const
{
    return this->status;
}
}
#endif