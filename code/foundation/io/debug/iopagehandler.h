#pragma once
#include "http/httprequesthandler.h"
namespace Debug
{
class IoPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(IoPageHandler);
public:
    IoPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
private:
    void ListDirectory(const Util::String& path, const Ptr<Http::HttpRequest>& request);
};
}