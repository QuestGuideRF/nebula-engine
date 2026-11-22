#pragma once
#include "memory/memory.h"
namespace Physics
{
class Allocator: public physx::PxAllocatorCallback
{
public:
    ~Allocator() {  }
    void* allocate(size_t size, const char* typeName, const char* filename, int line);
    void deallocate(void* ptr);
};
class ErrorCallback : public physx::PxErrorCallback
{
public:
    void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};
inline void*
Allocator::allocate(size_t size, const char * typeName, const char* filename, int line)
{
    void* buffer = Memory::Alloc(Memory::HeapType::PhysicsHeap, size);
    n_assert_fmt(buffer, "Не удалось выделить %s: %s, строка: %d", typeName, filename, line);
    return buffer;
}
inline void
Allocator::deallocate(void * ptr)
{
    Memory::Free(Memory::HeapType::PhysicsHeap, ptr);
}
}