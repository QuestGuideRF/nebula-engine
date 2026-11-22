#pragma once
#ifndef POSIX_POSIXCONSOLEHANDLER_H
#define POSIX_POSIXCONSOLEHANDLER_H
#include "io/consolehandler.h"
namespace Posix
{
class PosixConsoleHandler : public IO::ConsoleHandler
{
    __DeclareClass(PosixConsoleHandler);
public:
    PosixConsoleHandler();
    virtual void Print(const Util::String& s);
    virtual void Error(const Util::String& s);
    virtual void Warning(const Util::String& s);
    virtual void DebugOut(const Util::String& s);
    virtual bool HasInput();
    virtual Util::String GetInput();
private:
    FILE * stdoutHandle;
    FILE * stdinHandle;
    FILE * stderrHandle;
};
};
#endif