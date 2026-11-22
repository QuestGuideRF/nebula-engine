#pragma once
#if __WIN32__
#include "win32/win32process.h"
namespace System
{
typedef Win32::Win32Process Process;
}
#elif __linux__
#include "posix/posixprocess.h"
namespace System
{
typedef Posix::PosixProcess Process;
}
#else
#error "Система::Процесс не реализован на этой платформе!"
#endif