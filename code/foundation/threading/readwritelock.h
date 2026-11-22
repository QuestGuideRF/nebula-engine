#pragma once
#include "core/config.h"
#if (__WIN32__)
#include "threading/win32/win32readwritelock.h"
namespace Threading
{
class ReadWriteLock : public Win32::Win32ReadWriteLock
{
public:
    ReadWriteLock() : Win32::Win32ReadWriteLock()
    {};
};
}
#elif ( __linux__ || __OSX__ || __APPLE__ )
#include "threading/posix/posixreadwritelock.h"
namespace Threading
{
class ReadWriteLock : public Posix::PosixReadWriteLock
{
public:
    ReadWriteLock() : Posix::PosixReadWriteLock()
    {};
};
}
#else
#error "Threading::ReadWriteLock не реализован на этой платформе!"
#endif