#pragma once
#include "http/httprequesthandler.h"
namespace Debug
{
class StringAtomPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(StringAtomPageHandler);
public:
    StringAtomPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
};
}