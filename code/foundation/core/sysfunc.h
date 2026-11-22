#pragma once
#include "core/config.h"
#if __WIN32__
#include "core/win32/win32sysfunc.h"
namespace Core
{
class SysFunc : public Win32::SysFunc
{
};
}
#elif ( __OSX__ || __APPLE__ || __linux__ )
#include "core/posix/posixsysfunc.h"
namespace Core
{
class SysFunc : public Posix::SysFunc
{
};
}
#else
#error "Core::SysFunc не реализован на этой платформе!"
#endif