#pragma once
#include "core/types.h"
#include "threading/criticalsection.h"
namespace Win32
{
class Win32MemoryPool
{
public:
    Win32MemoryPool();
    ~Win32MemoryPool();
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
private:
    static const uint FreeBlockPattern = 0xFE;
    static const uint NewBlockPattern = 0xFD;
    static const int  BlockAlign = 16;
    Memory::HeapType heapType;
    uint blockSize;
    uint alignedBlockSize;
    uint poolSize;
    uint numBlocks;
    #if NEBULA_MEMORY_STATS
    volatile long allocCount;
    #endif
    SLIST_HEADER listHead;
    ubyte* poolStart;
    ubyte* poolEnd;
};
#if NEBULA_MEMORY_STATS
inline uint
Win32MemoryPool::GetAllocCount() const
{
    return this->allocCount;
}
#endif
inline uint
Win32MemoryPool::ComputeAlignedBlockSize(uint blockSize)
{
    uint blockSizeWithHeader = blockSize + sizeof(PSLIST_ENTRY);
    uint padding = (BlockAlign - (blockSizeWithHeader % BlockAlign)) % BlockAlign;
    return (blockSizeWithHeader + padding);
}
inline uint
Win32MemoryPool::GetNumBlocks() const
{
    return this->numBlocks;
}
inline uint
Win32MemoryPool::GetBlockSize() const
{
    return this->blockSize;
}
inline uint
Win32MemoryPool::GetAlignedBlockSize() const
{
    return this->alignedBlockSize;
}
inline uint
Win32MemoryPool::GetPoolSize() const
{
    return this->poolSize;
}
inline bool
Win32MemoryPool::IsPoolBlock(void* ptr) const
{
    return (ptr >= this->poolStart) && (ptr < this->poolEnd);
}
}