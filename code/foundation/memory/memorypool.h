#pragma once
#include "core/config.h"
#if (__WIN32__)
#include "memory/win32/win32memorypool.h"
namespace Memory
{
typedef Win32::Win32MemoryPool MemoryPool;
}
#elif ( __OSX__ || __APPLE__ || __linux__ )
#include "memory/posix/posixmemorypool.h"
namespace Memory
{
typedef Posix::PosixMemoryPool MemoryPool;
}
#else
#error "РЕАЛИЗУЙТЕ МЕНЯ!"
#endif