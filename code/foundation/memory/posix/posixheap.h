#pragma once
#ifndef POSIX_POSIXHEAP_H
#define POSIX_POSIXHEAP_H
#include "core/types.h"
#include "threading/interlocked.h"
#include "threading/criticalsection.h"
#include "util/array.h"
#include "util/list.h"
namespace Posix
{
class PosixHeap
{
public:
    static void Setup();
    PosixHeap(const char* name);
    ~PosixHeap();
    const char* GetName() const;
    void* Alloc(size_t size);
    void* Realloc(void* ptr, size_t newSize);
    void Free(void* ptr);
    #if NEBULA_MEMORY_STATS
    struct Stats
    {
        const char* name;
        int allocCount;
        int allocSize;
    };
    static Util::Array<Stats> GetAllHeapStats();
    static bool ValidateAllHeaps();
    bool ValidateHeap() const;
    int GetAllocCount() const;
    int GetAllocSize() const;
    #endif
private:
    PosixHeap();
    const char* name;
    #if NEBULA_MEMORY_STATS
    int volatile allocCount;
    int volatile allocSize;
    static Threading::CriticalSection*  criticalSection;
    static Util::List<PosixHeap*>* list;
    Util::List<PosixHeap*>::Iterator listIterator;
    #endif
};
inline const char*
PosixHeap::GetName() const
{
    n_assert(0 != this->name);
    return this->name;
}
__forceinline void*
PosixHeap::Alloc(size_t size)
{
    #if NEBULA_MEMORY_STATS
    Threading::Interlocked::Increment(this->allocCount);
    Threading::Interlocked::Add(this->allocSize, int(size));
    #endif
    return malloc(size);
}
__forceinline void*
PosixHeap::Realloc(void* ptr, size_t size)
{
    #if NEBULA_MEMORY_STATS
    size_t curSize = HeapSize(this->heap, 0, ptr);
    Threading::Interlocked::Add(this->allocSize, int(size - curSize));
    #endif
    return realloc(ptr, size);
}
__forceinline void
PosixHeap::Free(void* ptr)
{
    n_assert(0 != ptr);
    #if NEBULA_MEMORY_STATS
    size_t size = HeapSize(this->heap, 0, ptr);
    Threading::Interlocked::Add(this->allocSize, -int(size));
    Threading::Interlocked::Decrement(this->allocCount);
    #endif
    free(ptr);
}
}
#endif