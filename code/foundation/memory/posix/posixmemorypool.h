#pragma once
#include "core/types.h"
namespace Posix
{
class PosixMemoryPool
{
public:
    PosixMemoryPool();
    ~PosixMemoryPool();
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
PosixMemoryPool::GetAllocCount() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
#endif
inline uint
PosixMemoryPool::ComputeAlignedBlockSize(uint blockSize)
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
PosixMemoryPool::GetNumBlocks() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
PosixMemoryPool::GetBlockSize() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
PosixMemoryPool::GetAlignedBlockSize() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline uint
PosixMemoryPool::GetPoolSize() const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return 0;
}
inline bool
PosixMemoryPool::IsPoolBlock(void* ptr) const
{
    n_error("НЕ РЕАЛИЗОВАНО!\n");
    return false;
}
}