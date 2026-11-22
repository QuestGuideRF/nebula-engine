#pragma once
#ifndef DEBUG_DISPLAYPAGEHANDLER_H
#define DEBUG_DISPLAYPAGEHANDLER_H
#include "http/httprequesthandler.h"
#include "coregraphics/adapter.h"
#include "http/html/htmlpagewriter.h"
namespace Debug
{
class DisplayPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(DisplayPageHandler);
public:
    DisplayPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
private:
    void WriteAdapterInfo(CoreGraphics::Adapter::Code adapter, const Ptr<Http::HtmlPageWriter>& htmlWriter);
    Http::HttpStatus::Code WriteScreenshot(const Util::String& fileFormat, const Ptr<IO::Stream>& responseContentStream);
};
}
#endif