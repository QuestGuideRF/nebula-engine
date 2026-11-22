#pragma once
#include "http/httprequesthandler.h"
#include "io/historyconsolehandler.h"
namespace Debug
{
class ConsolePageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(ConsolePageHandler);
public:
    ConsolePageHandler();
    virtual ~ConsolePageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
private:
    Ptr<IO::HistoryConsoleHandler> historyConsoleHandler;
};
}