#pragma once
#ifndef HTTP_DEFAULTHTTPREQUESTHANDLER_H
#define HTTP_DEFAULTHTTPREQUESTHANDLER_H
#include "http/httprequesthandler.h"
namespace Http
{
class DefaultHttpRequestHandler : public HttpRequestHandler
{
    __DeclareClass(DefaultHttpRequestHandler);
public:
    virtual void HandleRequest(const Ptr<HttpRequest>& request);
};
}
#endif