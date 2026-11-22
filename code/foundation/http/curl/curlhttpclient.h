#pragma once
#include "core/config.h"
#include "core/refcounted.h"
#include "http/httpstatus.h"
#include "http/httpmethod.h"
#include "io/uri.h"
#include "io/stream.h"
#ifdef USE_CURL
namespace Http
{
class CurlHttpClient : public Core::RefCounted
{
    __DeclareClass(CurlHttpClient);
public:
    CurlHttpClient();
    virtual ~CurlHttpClient();
    void SetUserAgent(const Util::String& userAgent);
    const Util::String& GetUserAgent() const;
    bool Connect(const IO::URI& uri);
    void Disconnect();
    bool IsConnected() const;
    HttpStatus::Code SendRequest(HttpMethod::Code requestMethod, const IO::URI& uri, const Ptr<IO::Stream>& responseContentStream);
    HttpStatus::Code SendRequest(HttpMethod::Code requestMethod, const IO::URI& uri, const Util::String & body, const Ptr<IO::Stream>& responseContentStream);
private:
    Util::String userAgent;
    void* curlSession;
    char* curlErrorBuffer;
    static size_t CurlWriteDataCallback(char* ptr, size_t size, size_t nmemb, void* userData);
    static size_t CurlHeaderCallback(char* ptr, size_t size, size_t nmenb, void* userData);
};
inline void
CurlHttpClient::SetUserAgent(const Util::String& agent)
{
    this->userAgent = agent;
}
inline const Util::String&
CurlHttpClient::GetUserAgent() const
{
    return this->userAgent;
}
}
#endif