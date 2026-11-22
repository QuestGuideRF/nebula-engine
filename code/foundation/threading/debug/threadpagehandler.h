#pragma once
#ifndef DEBUG_THREADPAGEHANDLER_H
#define DEBUG_THREADPAGEHANDLER_H
#include "http/httprequesthandler.h"
namespace Debug
{
class ThreadPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(ThreadPageHandler);
public:
    ThreadPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
};
}
#endif