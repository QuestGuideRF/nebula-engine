#pragma once
#include "core/config.h"
#include "io/memorystream.h"
#include "http/httpclient.h"
namespace Http
{
class HttpStream : public IO::MemoryStream
{
    __DeclareClass(HttpStream);
public:
    HttpStream();
    virtual ~HttpStream();
    virtual bool Open();
    void SetRetries(int count);
    int GetRetries() const;
private:
    int retries;
};
inline void
HttpStream::SetRetries(int count)
{
    this->retries = count;
}
inline int
HttpStream::GetRetries() const
{
    return this->retries;
}
}