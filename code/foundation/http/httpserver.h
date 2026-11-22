#pragma once
#ifndef HTTP_HTTPSERVER_H
#define HTTP_HTTPSERVER_H
#include "core/refcounted.h"
#include "core/singleton.h"
#include "net/tcpserver.h"
#include "net/socket/ipaddress.h"
#include "io/textwriter.h"
#include "http/httpresponsewriter.h"
#include "http/httprequesthandler.h"
#include "http/defaulthttprequesthandler.h"
namespace Http
{
class HttpServer : public Core::RefCounted
{
    __DeclareClass(HttpServer);
    __DeclareSingleton(HttpServer);
public:
    HttpServer();
    virtual ~HttpServer();
    void SetPort(ushort p);
    ushort GetPort() const;
    void SetSingleThreadMode(bool b);
    bool IsSingleThreadMode() const;
    bool Open();
    void Close();
    bool IsOpen() const;
    void AttachRequestHandler(const Ptr<HttpRequestHandler>& h);
    void RemoveRequestHandler(const Ptr<HttpRequestHandler>& h);
    Util::Array<Ptr<HttpRequestHandler> > GetRequestHandlers() const;
    void OnFrame();
private:
    bool HandleHttpRequest(const Ptr<Net::TcpClientConnection>& clientConnection);
    bool BuildHttpResponse(const Ptr<Net::TcpClientConnection>& clientConnection, const Ptr<HttpRequest>& httpRequest);
    struct PendingRequest
    {
        Ptr<Net::TcpClientConnection> clientConnection;
        Ptr<HttpRequest> httpRequest;
    };
    Util::Dictionary<Util::String, Ptr<HttpRequestHandler> > requestHandlers;
    Ptr<DefaultHttpRequestHandler> defaultRequestHandler;
    Net::IpAddress ipAddress;
    Ptr<Net::TcpServer> tcpServer;
    Util::Array<PendingRequest> pendingRequests;
    bool isOpen;
    bool isSingleThreadMode;
};
inline bool
HttpServer::IsOpen() const
{
    return this->isOpen;
}
inline void
HttpServer::SetSingleThreadMode(bool b)
{
    n_assert(!this->isOpen);
    this->isSingleThreadMode = b;
}
inline bool
HttpServer::IsSingleThreadMode() const
{
    return this->isSingleThreadMode;
}
inline void
HttpServer::SetPort(ushort p)
{
    this->ipAddress.SetPort(p);
}
inline ushort
HttpServer::GetPort() const
{
    return this->ipAddress.GetPort();
}
inline Util::Array<Ptr<HttpRequestHandler> >
HttpServer::GetRequestHandlers() const
{
    return this->requestHandlers.ValuesAsArray();
}
}
#endif