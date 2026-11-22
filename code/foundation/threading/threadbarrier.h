#pragma once
#include "core/config.h"
#if (__WIN32__)
#include "threading/win32/win32threadbarrier.h"
namespace Threading
{
class ThreadBarrier : public Win32::Win32ThreadBarrier
{ };
}
#elif (__linux__ || __OSX__ || __APPLE__)
namespace Threading
{
#include "threading/posix/posixthreadbarrier.h"
class ThreadBarrier : public Posix::PosixThreadBarrier
{ };
}
#else
#error "Threading::ThreadBarrier не реализован на этой платформе!"
#endif