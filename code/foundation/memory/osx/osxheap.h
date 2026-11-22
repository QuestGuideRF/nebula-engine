#pragma once
#include "core/types.h"
namespace OSX
{
class OSXHeap
{
public:
    static void Setup();
    OSXHeap(const char* name, size_t initialSize=64 * 1024);
    ~OSXHeap();
    const char* GetName() const;
    void* Alloc(size_t size, size_t alignment=16);
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
    int GetAllocCount() const;
    int GetAllocSize() const;
#endif
private:
    OSXHeap();
    const char* name;
    malloc_zone_t* heapZone;
    #if NEBULA_MEMORY_STATS
    int volatile allocCount;
    int volatile allocSize;
    static Threading::CriticalSection* criticalSection;
    static Util::List<OSXHeap*>* list;
    Util::List<OSXHeap*>::Iterator listIterator;
    #endif
};
inline const char*
OSXHeap::GetName() const
{
    n_assert(0 != this->name);
    return this->name;
}
}