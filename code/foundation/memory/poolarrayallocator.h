#pragma once
#include "memory/memorypool.h"
namespace Memory
{
class PoolArrayAllocator
{
public:
    static const SizeT NumPools = 8;
    PoolArrayAllocator();
    ~PoolArrayAllocator();
    void Setup(const char* name, Memory::HeapType heapType, uint poolSizes[NumPools]);
    void* Alloc(SizeT size);
    void Free(void* ptr, SizeT size);
    void Free(void* ptr);
    const MemoryPool& GetMemoryPool(IndexT index) const;
    #if NEBULA_MEMORY_STATS
    void Dump();
    #endif
private:
    Memory::HeapType heapType;
    const char* name;
    MemoryPool memoryPools[NumPools];
};
}