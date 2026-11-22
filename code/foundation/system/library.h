#pragma once
#if __WIN32__
#include "win32/win32library.h"
namespace System
{
typedef Win32::Win32Library Library;
}
#else
#error "System::Library не реализована на этой платформе!"
#endif