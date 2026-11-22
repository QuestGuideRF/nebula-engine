#pragma once
#if __WIN32__
#include "system/win32/win32environment.h"
namespace System
{
typedef Win32::Win32Environment Environment;
}
#elif __LINUX__
#include "system/posix/posixenvironment.h"
namespace System
{
typedef Posix::PosixEnvironment Environment;
}
#else
#error "Система::Среда не реализована на этой платформе!"
#endif