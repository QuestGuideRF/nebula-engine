#pragma once
#include "http/httprequesthandler.h"
namespace Debug
{
class CorePageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(CorePageHandler);
public:
    CorePageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
};
}