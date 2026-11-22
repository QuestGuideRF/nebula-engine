//------------------------------------------------------------------------------
//  memory.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "memory.h"
#include "math/scalar.h"
namespace CoreGraphics
{

Util::Array<MemoryPool> Pools;
Util::Array<MemoryHeap> Heaps;
Threading::CriticalSection AllocationLock;

//------------------------------------------------------------------------------
/**
*/
Alloc
MemoryPool::AllocateMemory(uint alignment, uint size)
{
   // if size is too big, allocate a unique block for it
    if ((size + alignment) > this->blockSize)
        return this->AllocateExclusiveBlock(alignment, size);

    for (IndexT blockIndex = 0; blockIndex < this->blocks.Size(); blockIndex++)
    {
        // if the block has been dealloced, use it
        if (this->blocks[blockIndex] == DeviceMemory(0))
            break;

        Memory::RangeAllocation alloc = this->allocators[blockIndex].Alloc(size, alignment);

        // Если блок не помещается в нашу память, переходим к следующему блоку.
        if (alloc.offset == Memory::RangeAllocation::OOM)
            continue;
     
        Alloc ret{ this->blocks[blockIndex], alloc.offset, size, alloc.node, this->memoryType, (uint)blockIndex };
        return ret;
    }

    n_assert(this->size + this->blockSize <= this->maxSize);
    this->size += this->blockSize;

    // Ни один существующий блок не может соответствовать нашему распределению, поэтому создайте новый блок.
    uint id = this->blockPool.Alloc();
    if (id >= (uint)this->blockMappedPointers.Size())
    {
        // Блок совершенно новый, освободите место в наших массивах для данных.
        this->blockMappedPointers.Append(nullptr);
        this->allocators.Append(Memory::RangeAllocator{ (uint)this->blockSize, 256} );

        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);
        this->blocks.Append(mem);
    }
    else
    {
        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);
        this->allocators[id] = Memory::RangeAllocator{ (uint)this->blockSize, 256 };
        this->blocks[id] = mem;
    }
    Memory::RangeAllocation alloc = this->allocators[id].Alloc(size, alignment);
    Alloc ret{ this->blocks[id], alloc.offset, size, alloc.node, this->memoryType, id };
    return ret;
}

//------------------------------------------------------------------------------
/**
    Освобождает память. Если блок графического процессора был освобожден, эта функция возвращает true.
*/
bool 
MemoryPool::DeallocateMemory(const Alloc& alloc)
{
    bool deleteBlock = true;

    // Выделенные блоки будут использовать только nodeIndex 0xFFFFFFFF.
    if (alloc.nodeIndex != DedicatedBlockNodeIndex)
    {
        Memory::RangeAllocator& allocator = this->allocators[alloc.blockIndex];
        allocator.Dealloc(Memory::RangeAllocation{ .offset = (uint)alloc.offset, .size = 0, .node = alloc.nodeIndex });
        deleteBlock = allocator.Empty();
    }
    if (deleteBlock)
    {
        this->DestroyBlock(this->blocks[alloc.blockIndex]);
        this->blockPool.Dealloc(alloc.blockIndex);
        this->blocks[alloc.blockIndex] = DeviceMemory(0);
        this->blockMappedPointers[alloc.blockIndex] = nullptr;
    }

    return true;
}

//------------------------------------------------------------------------------
/**
*/
void 
MemoryPool::Clear()
{
    for (IndexT i = 0; i < this->blocks.Size(); i++)
        if (this->blocks[i] != DeviceMemory(0))
            this->DestroyBlock(this->blocks[i]);

    this->blocks.Clear();
    this->allocators.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void* 
MemoryPool::GetMappedMemory(const Alloc& alloc)
{
    return (char*)this->blockMappedPointers[alloc.blockIndex] + alloc.offset;
}

//------------------------------------------------------------------------------
/**
*/
Alloc
MemoryPool::AllocateExclusiveBlock(DeviceSize alignment, DeviceSize size)
{
     // сохранить старый размер блока и сбросить его после выделения блока
    uint oldSize = this->blockSize;
    this->blockSize = size;

    uint id = this->blockPool.Alloc();
    if (id >= (uint)this->blockMappedPointers.Size())
    {
        this->blockMappedPointers.Append(nullptr);
        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);

        this->blocks.Append(mem);
        this->allocators.Append(Memory::RangeAllocator{ 0, 0 });
    }
    else
    {
        this->blockMappedPointers[id] = nullptr;
        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);

        this->blocks[id] = mem;
        this->allocators[id] = Memory::RangeAllocator{ 0, 0 };
    }
    this->blockSize = oldSize;

    n_warning("[Память графического процессора] Размер выделения %ld превышает размер блока %ld, будет выделен выделенный блок памяти\n", size, this->blockSize);
    Alloc ret{ this->blocks[id], 0, size, DedicatedBlockNodeIndex, this->memoryType, id };
    return ret;
}

} // пространство имен CoreGraphics
