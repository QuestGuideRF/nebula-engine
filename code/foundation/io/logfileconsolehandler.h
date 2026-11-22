#pragma once
#include "io/consolehandler.h"
#include "io/stream.h"
#include "io/textwriter.h"
namespace IO
{
class LogFileConsoleHandler : public ConsoleHandler
{
    __DeclareClass(LogFileConsoleHandler);
public:
    LogFileConsoleHandler();
    virtual ~LogFileConsoleHandler();
    virtual void Open();
    virtual void Close();
    virtual void Print(const Util::String& s);
    virtual void Error(const Util::String& s);
    virtual void Warning(const Util::String& s);
    virtual void DebugOut(const Util::String& s);
private:
    Ptr<Stream> stream;
    Ptr<TextWriter> textWriter;
};
}