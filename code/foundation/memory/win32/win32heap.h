#pragma once
#include "core/types.h"
#include "threading/interlocked.h"
#include "threading/criticalsection.h"
#include "util/array.h"
#include "util/list.h"
namespace Win32
{
class Win32Heap
{
public:
    static void Setup();
    Win32Heap(const char* name, size_t initialSize=0, size_t maxSize=0);
    ~Win32Heap();
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
    void DumpLeaks();
    static void DumpLeaksAllHeaps();
    long GetAllocCount() const;
    long GetAllocSize() const;
    static void DumpHeapMemoryLeaks(const char* heapName, HANDLE hHeap);
    #endif
private:
    Win32Heap();
    HANDLE heap;
    const char* name;
    #if NEBULA_MEMORY_STATS
    long volatile allocCount;
    long volatile allocSize;
    static Threading::CriticalSection*  criticalSection;
    static Util::List<Win32Heap*>* list;
    Util::List<Win32Heap*>::Iterator listIterator;
    #endif
};
inline const char*
Win32Heap::GetName() const
{
    n_assert(0 != this->name);
    return this->name;
}
__forceinline void*
Win32Heap::Alloc(size_t size)
{
    #if NEBULA_MEMORY_STATS
    Threading::Interlocked::Increment((volatile int*)&this->allocCount);
    Threading::Interlocked::Add((volatile int*)&this->allocSize, int(size + 16));
    #endif
    void* ptr = Memory::__HeapAlloc16(this->heap, HEAP_GENERATE_EXCEPTIONS, size);
    return ptr;
}
__forceinline void*
Win32Heap::Realloc(void* ptr, size_t size)
{
    #if NEBULA_MEMORY_STATS
    size_t curSize = Memory::__HeapSize16(this->heap, 0, ptr);
    Threading::Interlocked::Add((volatile int*)&this->allocSize, int(size - curSize + 16));
    #endif
    void* newPtr = Memory::__HeapReAlloc16(this->heap, HEAP_GENERATE_EXCEPTIONS, ptr, size);
    return newPtr;
}
__forceinline void
Win32Heap::Free(void* ptr)
{
    n_assert(0 != ptr);
    #if NEBULA_MEMORY_STATS
    size_t size = Memory::__HeapSize16(this->heap, 0, ptr);
    Threading::Interlocked::Add((volatile int*)&this->allocSize, -int(size));
    Threading::Interlocked::Decrement((volatile int*)&this->allocCount);
    #endif
    BOOL success = Memory::__HeapFree16(this->heap, 0, ptr);
    n_assert(0 != success);
}
}