#pragma once
#if __WIN32__
#include "win32/win32applauncher.h"
namespace ToolkitUtil
{
typedef ToolkitUtil::Win32AppLauncher AppLauncher;
}
#elif __LINUX__
#include "toolkit-common/posix/posixapplauncher.h"
namespace ToolkitUtil
{
typedef ToolkitUtil::PosixAppLauncher AppLauncher;
}
#else
#error "ToolkitUtil::AppLauncher не реализован на этой платформе!"
#endif