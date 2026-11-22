#pragma once
#if __WIN32__
#include "system/win32/win32registry.h"
namespace System
{
typedef Win32::Win32Registry NebulaSettings;
}
#elif __LINUX__
#include "system/posix/posixsettings.h"
namespace System
{
typedef Posix::PosixSettings NebulaSettings;
}
#else
#error "System::NebulaSettings не реализована на этой платформе!"
#endif