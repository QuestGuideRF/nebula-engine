#pragma once
#ifndef DEBUG_TEXTUREPAGEHANDLER_H
#define DEBUG_TEXTUREPAGEHANDLER_H
#include "http/httprequesthandler.h"
#include "resources/resourceid.h"
namespace Debug
{
class TexturePageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(TexturePageHandler);
public:
    TexturePageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
private:
    Http::HttpStatus::Code HandleImageRequest(const Util::Dictionary<Util::String,Util::String>& query, const Ptr<IO::Stream>& responseStream);
    Http::HttpStatus::Code HandleTextureInfoRequest(const Util::String& resId, const Ptr<IO::Stream>& responseContentStream);
};
}
#endif