#pragma once
#ifndef HTTP_HTTPSERVERPROXY_H
#define HTTP_HTTPSERVERPROXY_H
#include "core/refcounted.h"
#include "core/singleton.h"
#include "http/httprequesthandler.h"
namespace Http
{
class HttpServerProxy : public Core::RefCounted
{
    __DeclareClass(HttpServerProxy);
    __DeclareSingleton(HttpServerProxy);
public:
    HttpServerProxy();
    virtual ~HttpServerProxy();
    void Open();
    void Close();
    bool IsOpen() const;
    void AttachRequestHandler(const Ptr<Http::HttpRequestHandler>& h);
    void RemoveRequestHandler(const Ptr<Http::HttpRequestHandler>& h);
    void HandlePendingRequests();
private:
    Util::Array<Ptr<HttpRequestHandler> > requestHandlers;
    bool isOpen;
};
inline bool
HttpServerProxy::IsOpen() const
{
    return this->isOpen;
}
}
#endif