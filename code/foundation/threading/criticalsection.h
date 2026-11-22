#pragma once
#include "core/config.h"
#if (__WIN32__)
#include "threading/win32/win32criticalsection.h"
namespace Threading
{
class CriticalSection : public Win32::Win32CriticalSection
{ };
}
#elif ( __OSX__ || __APPLE__ || __linux__ )
#include "threading/posix/posixcriticalsection.h"
namespace Threading
{
class CriticalSection : public Posix::PosixCriticalSection
{ };
}
#else
#error "Threading::CriticalSection не реализован на этой платформе!"
#endif
namespace Threading
{
struct CriticalScope
{
    CriticalScope(Threading::CriticalSection* section)
        : section(section)
    {
        this->section->Enter();
    }
    ~CriticalScope()
    {
        this->section->Leave();
        this->section = nullptr;
    }
    Threading::CriticalSection* section;
};
}