#pragma once
#include "interface/interfacehandlerbase.h"
#include "debug/debugserver.h"
#include "http/httpserverproxy.h"
namespace Debug
{
class DebugHandler : public Interface::InterfaceHandlerBase
{
    __DeclareClass(DebugHandler);
public:
    DebugHandler();
    virtual ~DebugHandler();
    virtual void Open();
    virtual void Close();
    virtual void DoWork();
private:
    Ptr<DebugServer> debugServer;
    Ptr<Http::HttpServerProxy> httpServerProxy;
};
}