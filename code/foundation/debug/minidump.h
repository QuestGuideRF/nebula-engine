#pragma once
#if __WIN32__
#include "debug/win32/win32minidump.h"
namespace Debug
{
class MiniDump : public Win32::Win32MiniDump
{ };
}
#endif