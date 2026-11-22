#pragma once
#ifndef DEBUG_SVGTESTPAGEHANDLER_H
#define DEBUG_SVGTESTPAGEHANDLER_H
#include "http/httprequesthandler.h"
namespace Debug
{
class SvgTestPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(SvgTestPageHandler);
public:
    SvgTestPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
private:
    bool TestShapeRendering(const Ptr<Http::HttpRequest>& request);
    bool TestLineChartRendering(const Ptr<Http::HttpRequest>& request);
};
}
#endif