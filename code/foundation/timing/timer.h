#pragma once
#include "core/config.h"
#if __WIN32__
#include "timing/win32/win32timer.h"
namespace Timing
{
class Timer : public Win32::Win32Timer
{ };
}
#elif __linux__
#include "timing/posix/posixtimer.h"
namespace Timing
{
class Timer : public Posix::PosixTimer
{ };
}
#else
#error "Timing::Timer не реализован на этой платформе!"
#endif