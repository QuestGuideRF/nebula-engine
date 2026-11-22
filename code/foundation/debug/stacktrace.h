#pragma once
#include "core/config.h"
#if __WIN32__
#include "debug/win32/win32stacktrace.h"
namespace Debug
{
    class StackTrace : public Win32::Win32StackTrace
    { };
}
#endif