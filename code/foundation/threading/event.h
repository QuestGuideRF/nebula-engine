#pragma once
#include "core/config.h"
#if (__WIN32__)
#include "threading/win32/win32event.h"
namespace Threading
{
class Event : public Win32::Win32Event
{
public:
    Event(bool manualReset=false) : Win32Event(manualReset) {};
};
}
#elif __linux__
#include "threading/linux/linuxevent.h"
namespace Threading
{
class Event : public Linux::LinuxEvent
{
public:
    Event(bool manualReset=false) : LinuxEvent(manualReset) {};
};
}
#elif ( __OSX__ || __APPLE__ )
#include "threading/posix/posixevent.h"
namespace Threading
{
class Event : public Posix::PosixEvent
{
public:
    Event(bool manualReset=false) : PosixEvent(manualReset) {};
};
}
#else
#error "Threading::Событие не реализовано на этой платформе!"
#endif
namespace Threading
{
class EventWithManualReset : public Event
{
public:
    EventWithManualReset() : Event(true) {};
};
}