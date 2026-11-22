#pragma once
#include "core/config.h"
#if (__WIN32__)
#include "memory/win32/win32heap.h"
namespace Memory
{
typedef Win32::Win32Heap Heap;
}
#elif ( __OSX__ || __APPLE__ || __linux__ )
#include "memory/posix/posixheap.h"
namespace Memory
{
typedef Posix::PosixHeap Heap;
}
#else
#error "РЕАЛИЗУЙТЕ МЕНЯ!"
#endif