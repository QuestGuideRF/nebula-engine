#pragma once
#include "core/refcounted.h"
#include "dynui/console/imguiconsole.h"
#include "io/consolehandler.h"
#include "util/string.h"
namespace Dynui
{
class ImguiConsoleHandler : public IO::ConsoleHandler
{
    __DeclareClass(ImguiConsoleHandler);
public:
    ImguiConsoleHandler();
    virtual ~ImguiConsoleHandler();
    void Setup();
    void Discard();
    virtual void Open();
    virtual void Close();
    virtual void Print(const Util::String& s);
    virtual void Error(const Util::String& s);
    virtual void Warning(const Util::String& s);
    virtual void DebugOut(const Util::String& s);
};
}