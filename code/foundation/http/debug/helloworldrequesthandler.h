#pragma once
#include "http/httprequesthandler.h"
namespace Debug
{
class HelloWorldRequestHandler : public Http::HttpRequestHandler
{
    __DeclareClass(HelloWorldRequestHandler);
public:
    HelloWorldRequestHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
};
}