#pragma once
#include "threading/criticalsection.h"
#include "util/array.h"
#include "ids/idpool.h"
#include "memory/rangeallocator.h"
#if __VULKAN__
#include "vk/vkloader.h"
#endif
namespace CoreGraphics
{
#if __VULKAN__
typedef VkDeviceSize DeviceSize;
typedef VkDeviceMemory DeviceMemory;
typedef VkDeviceAddress DeviceAddress;
#else
#error "coregraphics/memory.h не поддерживается средством рендеринга"
#endif
enum MemoryPoolType
{
    MemoryPool_DeviceLocal,
    MemoryPool_HostLocal,
    MemoryPool_HostCached,
    MemoryPool_DeviceAndHost,
    NumMemoryPoolTypes
};
struct Alloc
{
    DeviceMemory mem;
    DeviceSize offset;
    DeviceSize size;
    uint nodeIndex;
    uint poolIndex;
    uint blockIndex;
};
struct AllocRange
{
    DeviceSize offset;
    DeviceSize size;
};
struct MemoryHeap
{
    DeviceSize space;
};
struct MemoryPool
{
    Alloc AllocateMemory(uint alignment, uint size);
    bool DeallocateMemory(const Alloc& alloc);
    void Clear();
    void* GetMappedMemory(const Alloc& alloc);
    DeviceSize blockSize;
    uint memoryType;
    Ids::IdPool blockPool;
    Util::Array<DeviceMemory> blocks;
    Util::Array<Memory::RangeAllocator> allocators;
    Util::Array<void*> blockMappedPointers;
    DeviceSize size;
    const char* budgetCounter;
    DeviceSize maxSize;
    bool mapMemory;
    MemoryHeap* heap;
private:
    static constexpr uint DedicatedBlockNodeIndex = 0xFFFFFFFF;
    DeviceMemory CreateBlock(void** outMappedPtr);
    void DestroyBlock(DeviceMemory mem);
    Alloc AllocateExclusiveBlock(DeviceSize alignment, DeviceSize size);
};
extern Util::Array<MemoryPool> Pools;
extern Util::Array<MemoryHeap> Heaps;
extern Threading::CriticalSection AllocationLock;
void SetupMemoryPools(
    DeviceSize deviceLocalMemory,
    DeviceSize hostLocalMemory,
    DeviceSize hostToDeviceMemory,
    DeviceSize deviceToHostMemory);
void DiscardMemoryPools(VkDevice dev);
void FreeMemory(const CoreGraphics::Alloc& alloc);
void* GetMappedMemory(const CoreGraphics::Alloc& alloc);
}