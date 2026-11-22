#pragma once
#include "core/config.h"
#include "core/debug.h"
namespace Memory
{
__forceinline void
Copy(const void* from, void* to, size_t numBytes)
{
    if (numBytes > 0)
    {
        n_assert(0 != from);
        n_assert(0 != to);
        n_assert(from != to);
        CopyMemory(to, from, numBytes);
    }
}
template <typename T>
__forceinline void
CopyElements(const T* from, T* to, size_t numElements)
{
    if (numElements > 0)
    {
        n_assert(0 != from);
        n_assert(0 != to);
        n_assert(from != to);
        CopyMemory((void*)to, (const void*)from, numElements * sizeof(T));
    }
}
__forceinline void
Move(const void* from, void* to, size_t numBytes)
{
    if (numBytes > 0)
    {
        n_assert(0 != from);
        n_assert(0 != to);
        n_assert(from != to);
        MoveMemory(to, from, numBytes);
    }
}
template <typename T>
__forceinline void
MoveElements(const T* from, T* to, size_t numElements)
{
    if (numElements > 0)
    {
        n_assert(0 != from);
        n_assert(0 != to);
        n_assert(from != to);
        MoveMemory((void*)to, (const void*)from, numElements * sizeof(T));
    }
}
__forceinline void
CopyToGraphicsMemory(const void* from, void* to, size_t numBytes)
{
    Memory::Copy(from, to, numBytes);
}
__forceinline void
Clear(void* ptr, size_t numBytes)
{
    ZeroMemory(ptr, numBytes);
}
__forceinline void
Fill(void* ptr, size_t numBytes, unsigned char value)
{
    FillMemory(ptr, numBytes, value);
}
}