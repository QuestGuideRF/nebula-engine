#pragma once
#include "io/consolehandler.h"
namespace OSX
{
class OSXConsoleHandler : public IO::ConsoleHandler
{
    __DeclareClass(OSXConsoleHandler);
public:
    virtual void Print(const Util::String& s);
    virtual void Error(const Util::String& s);
    virtual void Warning(const Util::String& s);
    virtual void Confirm(const Util::String& s);
    virtual void DebugOut(const Util::String& s);
    virtual bool HasInput();
    virtual Util::String GetInput();
};
}