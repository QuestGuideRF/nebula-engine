#pragma once
#if (__WIN32__)
#include "io/win32/win32fswrapper.h"
namespace IO
{
class FSWrapper : public Win32::Win32FSWrapper
{ };
}
#elif ( __OSX__ || __APPLE__ || __linux__ )
#include "io/posix/posixfswrapper.h"
namespace IO
{
class FSWrapper : public Posix::PosixFSWrapper
{ };
}
#else
#error "Класс FSWrapper не реализован на этой платформе!"
#endif