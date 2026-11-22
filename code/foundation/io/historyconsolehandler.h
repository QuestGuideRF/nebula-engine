#pragma once
#include "io/consolehandler.h"
#include "util/ringbuffer.h"
namespace IO
{
class HistoryConsoleHandler : public ConsoleHandler
{
    __DeclareClass(HistoryConsoleHandler);
public:
    HistoryConsoleHandler();
    void SetHistorySize(SizeT numLines);
    SizeT GetHistorySize() const;
    const Util::RingBuffer<Util::String>& GetHistory() const;
    virtual void Print(const Util::String& s);
    virtual void Error(const Util::String& s);
    virtual void Warning(const Util::String& s);
    virtual void DebugOut(const Util::String& s);
private:
    Util::RingBuffer<Util::String> history;
};
inline void
HistoryConsoleHandler::SetHistorySize(SizeT numLines)
{
    this->history.SetCapacity(numLines);
}
inline SizeT
HistoryConsoleHandler::GetHistorySize() const
{
    return this->history.Size();
}
inline const Util::RingBuffer<Util::String>&
HistoryConsoleHandler::GetHistory() const
{
    return this->history;
}
}