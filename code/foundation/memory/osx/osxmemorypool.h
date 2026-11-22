#pragma once
#include "core/types.h"
namespace OSX
{
class OSXMemoryPool
{
public:
    OSXMemoryPool();
    ~OSXMemoryPool();
    static uint ComputeAlignedBlockSize(uint blockSize);
    void Setup(Memory::HeapType heapType, uint blockSize, uint numBlocks);
    void* Alloc();
    void Free(void* ptr);
    bool IsPoolBlock(void* ptr) const;
    uint GetNumBlocks() const;
    uint GetBlockSize() const;
    uint GetAlignedBlockSize() const;
    uint GetPoolSize() const;
#if NEBULA_MEMORY_STATS
    uint GetAllocCount() const;
#endif
};
#if NEBULA_MEMORY_STATS
inline uint
OSXMemoryPool::GetAllocCount() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
#endif
inline uint
OSXMemoryPool::ComputeAlignedBlockSize(uint blockSize)
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
OSXMemoryPool::GetNumBlocks() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
OSXMemoryPool::GetBlockSize() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
OSXMemoryPool::GetAlignedBlockSize() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
OSXMemoryPool::GetPoolSize() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline bool
OSXMemoryPool::IsPoolBlock(void* ptr) const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return false;
}
}