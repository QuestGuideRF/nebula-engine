#pragma once
#include "core/types.h"
namespace Win32
{
class Win32Event
{
public:
    Win32Event(bool manualReset=false);
    Win32Event(Win32Event&& rhs);
    ~Win32Event();
    void Signal();
    void Reset();
    void Wait() const;
    bool WaitTimeout(int ms) const;
    bool Peek() const;
    bool IsManual() const;
private:
    bool manual;
    HANDLE event;
};
inline
Win32Event::Win32Event(bool manualReset)
{
    this->event = CreateEvent(NULL, manualReset, FALSE, NULL);
    this->manual = manualReset;
    n_assert(0 != this->event);
}
inline
Win32Event::Win32Event(Win32Event&& rhs)
{
    if (this->event)
        CloseHandle(this->event);
    this->event = rhs.event;
    this->manual = rhs.manual;
    rhs.event = nullptr;
}
inline
Win32Event::~Win32Event()
{
    if (this->event != nullptr)
    {
        CloseHandle(this->event);
        this->event = nullptr;
    }
}
inline void
Win32Event::Signal()
{
    SetEvent(this->event);
}
inline void
Win32Event::Reset()
{
    ResetEvent(this->event);
}
inline void
Win32Event::Wait() const
{
    WaitForSingleObject(this->event, INFINITE);
}
inline bool
Win32Event::WaitTimeout(int timeoutInMilliSec) const
{
    DWORD res = WaitForSingleObject(this->event, timeoutInMilliSec);
    return (WAIT_TIMEOUT == res) ? false : true;
}
inline bool
Win32Event::Peek() const
{
    DWORD res = WaitForSingleObject(this->event, 0);
    return (WAIT_TIMEOUT == res) ? false : true;
}
inline bool
Win32Event::IsManual() const
{
    return this->manual;
}
}