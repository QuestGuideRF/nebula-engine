#pragma once
#include "io/streamwriter.h"
#include "http/httpmethod.h"
namespace Http
{
class HttpRequestWriter : public IO::StreamWriter
{
    __DeclareClass(HttpRequestWriter);
public:
    HttpRequestWriter();
    void SetMethod(HttpMethod::Code httpMethod);
    void SetURI(const IO::URI& uri);
    void SetUserAgent(const Util::String& userAgent);
    bool WriteRequestHeader();
    bool WriteRequestHeaderWithBody(const Util::String & body);
private:
    HttpMethod::Code httpMethod;
    IO::URI uri;
    Util::String userAgent;
};
inline void
HttpRequestWriter::SetMethod(HttpMethod::Code m)
{
    this->httpMethod = m;
}
inline void
HttpRequestWriter::SetURI(const IO::URI& u)
{
    this->uri = u;
}
inline void
HttpRequestWriter::SetUserAgent(const Util::String& agent)
{
    this->userAgent = agent;
}
}