#pragma once
#include "core/config.h"
#include "core/refcounted.h"
#include "net/tcpclient.h"
#include "http/httpstatus.h"
#include "http/httpmethod.h"
#include "io/uri.h"
#include "io/stream.h"
namespace Http
{
class NebulaHttpClient : public Core::RefCounted
{
    __DeclareClass(NebulaHttpClient);
public:
    NebulaHttpClient();
    virtual ~NebulaHttpClient();
    void SetUserAgent(const Util::String& userAgent);
    const Util::String& GetUserAgent() const;
    bool Connect(const IO::URI& uri);
    void Disconnect();
    bool IsConnected() const;
    HttpStatus::Code SendRequest(HttpMethod::Code requestMethod, const IO::URI& uri, const Ptr<IO::Stream>& responseContentStream);
    HttpStatus::Code SendRequest(HttpMethod::Code requestMethod, const IO::URI& uri, const Util::String & body, const Ptr<IO::Stream>& responseContentStream);
private:
    Util::String userAgent;
    Ptr<Net::TcpClient> tcpClient;
};
inline void
NebulaHttpClient::SetUserAgent(const Util::String& agent)
{
    this->userAgent = agent;
}
inline const Util::String&
NebulaHttpClient::GetUserAgent() const
{
    return this->userAgent;
}
}