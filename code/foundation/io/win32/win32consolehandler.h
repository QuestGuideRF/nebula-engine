#pragma once
#ifndef WIN32_WIN32CONSOLEHANDLER_H
#define WIN32_WIN32CONSOLEHANDLER_H
#include "io/consolehandler.h"
namespace Win32
{
class Win32ConsoleHandler : public IO::ConsoleHandler
{
    __DeclareClass(Win32ConsoleHandler);
public:
    Win32ConsoleHandler();
    virtual void Print(const Util::String& s);
    virtual void Error(const Util::String& s);
    virtual void Warning(const Util::String& s);
    virtual void Confirm(const Util::String& s);
    virtual void DebugOut(const Util::String& s);
    virtual bool HasInput();
    virtual Util::String GetInput();
private:
    HANDLE stdoutHandle;
    HANDLE stdinHandle;
    HANDLE stderrHandle;
};
};
#endif