#pragma once
#ifndef HTTP_HTTPREQUESTHANDLER_H
#define HTTP_HTTPREQUESTHANDLER_H
#include "core/refcounted.h"
#include "http/httprequest.h"
#include "util/string.h"
#include "threading/safequeue.h"
namespace Http
{
class HttpRequestHandler : public Core::RefCounted
{
    __DeclareClass(HttpRequestHandler);
public:
    HttpRequestHandler();
    virtual ~HttpRequestHandler();
    const Util::String& GetName() const;
    const Util::String& GetDesc() const;
    const Util::String& GetRootLocation() const;
protected:
    friend class HttpServer;
    friend class HttpServerProxy;
    virtual void HandleRequest(const Ptr<HttpRequest>& request);
    void HandlePendingRequests();
    void PutRequest(const Ptr<HttpRequest>& httpRequest);
    void SetName(const Util::String& n);
    void SetDesc(const Util::String& d);
    void SetRootLocation(const Util::String& l);
    Util::String name;
    Util::String desc;
    Util::String rootLocation;
    Threading::SafeQueue<Ptr<HttpRequest> > pendingRequests;
    Util::Array<Ptr<HttpRequest> > curWorkRequests;
};
inline void
HttpRequestHandler::SetName(const Util::String& n)
{
    this->name = n;
}
inline const Util::String&
HttpRequestHandler::GetName() const
{
    return this->name;
}
inline void
HttpRequestHandler::SetDesc(const Util::String& d)
{
    this->desc = d;
}
inline const Util::String&
HttpRequestHandler::GetDesc() const
{
    return this->desc;
}
inline void
HttpRequestHandler::SetRootLocation(const Util::String& l)
{
    this->rootLocation = l;
}
inline const Util::String&
HttpRequestHandler::GetRootLocation() const
{
    return this->rootLocation;
}
}
#endif