#pragma once
#include "core/config.h"
#if __WIN32__
#include "util/win32/win32guid.h"
namespace Util
{
typedef Win32::Win32Guid Guid;
}
#elif __OSX__
#include "util/osx/osxguid.h"
namespace Util
{
typedef OSX::OSXGuid Guid;
}
#elif ( __APPLE__ || __linux__ )
#include "util/posix/posixguid.h"
namespace Util
{
typedef Posix::PosixGuid Guid;
}
#else
#error "Util::Guid не реализован на этой платформе!"
#endif