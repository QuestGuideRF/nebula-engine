#pragma once
#ifndef DEBUG_MEMORYPAGEHANDLER_H
#define DEBUG_MEMORYPAGEHANDLER_H
#include "http/httprequesthandler.h"
namespace Debug
{
class MemoryPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(MemoryPageHandler);
public:
    MemoryPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
};
}
#endif