//------------------------------------------------------------------------------
//  poolarrayallocator.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "memory/poolarrayallocator.h"
#include "util/string.h"

namespace Memory
{

//------------------------------------------------------------------------------
/**
*/
PoolArrayAllocator::PoolArrayAllocator() :
    heapType(InvalidHeapType),
    name(0)
{
    Memory::Clear(this->memoryPools, sizeof(this->memoryPools));
}

//------------------------------------------------------------------------------
/**
*/
PoolArrayAllocator::~PoolArrayAllocator()
{
    // проверить утечку памяти
    #if NEBULA_MEMORY_STATS
    this->Dump();
    #endif
}

//------------------------------------------------------------------------------
/**
*/
void
PoolArrayAllocator::Setup(const char* name_, Memory::HeapType heapType_, uint poolSizes[NumPools])
{
    n_assert(0 != name_);
    n_assert(0 == this->name);

    // ПРИМЕЧАНИЕ. name_ должно быть статической строкой!
    this->name = name_;
    this->heapType = heapType_;

    // При настройке пулов памяти мы принимаем несколько предположений относительно пула памяти:
    // - есть 4 байта служебных данных 
    // - выделения выравниваются по 16 байтам
    // thus:
    // мы начинаем с 28 байт для наименьшего размера блока, это дает нам
    // 32-байтовые блоки в пуле памяти, и для каждого последующего
    // размер блока прибавляем 32, таким образом получаем следующий блок
    // размеры для 8 бассейнов:
    // 28 -> 60 -> 92 -> 124 -> 156 -> 188 -> 220 -> 252
    SizeT curBlockSize = 28;
    IndexT i;
    for (i = 0; i < NumPools; i++)
    {
        n_assert(poolSizes[i] > 0);
        SizeT alignedBlockSize = MemoryPool::ComputeAlignedBlockSize(curBlockSize);
        n_assert((curBlockSize + 4) == alignedBlockSize);
        n_assert(alignedBlockSize == (32 * (i + 1)));
        SizeT curNumBlocks = poolSizes[i] / alignedBlockSize;
        this->memoryPools[i].Setup(this->heapType, curBlockSize, curNumBlocks);
        curBlockSize += 32;
    }
}

//------------------------------------------------------------------------------
/**
*/
void*
PoolArrayAllocator::Alloc(SizeT size)
{
    IndexT poolIndex = (size + 3) >> 5;
    if (poolIndex < NumPools)
    {
#if NEBULA_DEBUG
        n_assert(uint(size) <= memoryPools[poolIndex].GetBlockSize());
#endif
        void* ptr = this->memoryPools[poolIndex].Alloc();
        if (0 == ptr)
        {
            n_error("PoolArrayAllocator '%s': пул с размером блока '%d' заполнен!\n",
                this->name, this->memoryPools[poolIndex].GetBlockSize());
        }
        return ptr;
    }
    else
    {
        // размер слишком велик, необходимо выделить непосредственно из кучи
#if NEBULA_DEBUG
        // n_printf("ВНИМАНИЕ: выделение размера '%d' в PoolArrayAllocator '%s' происходит непосредственно в куче!\n", size, this->name);        
#endif
        return Memory::Alloc(this->heapType, size);
    }
}

//------------------------------------------------------------------------------
/**
    This is the faster version to free a memory block, if the caller knows
    the size of the memory block we can compute the memory pool index
    whithout asking each pool whether the pointer is owned by this pool.
*/
void
PoolArrayAllocator::Free(void* ptr, SizeT size)
{
    IndexT poolIndex = (size + 3) >> 5;
    if (poolIndex < NumPools)
    {
        #if NEBULA_DEBUG
        n_assert(this->memoryPools[poolIndex].IsPoolBlock(ptr));
        #endif
        this->memoryPools[poolIndex].Free(ptr);
    }
    else
    {
        Memory::Free(this->heapType, ptr);
    }
}

//------------------------------------------------------------------------------
/**
    This is the slower version to free a memory block. Worst case is, that
    the allocator needs to check each memory pool whether the pointer is
    owned by the pool.
*/
void
PoolArrayAllocator::Free(void* ptr)
{
    IndexT i;
    for (i = 0; i < NumPools; i++)
    {
        if (this->memoryPools[i].IsPoolBlock(ptr))
        {
            this->memoryPools[i].Free(ptr);
            return;
        }
    }
    // провал: должен быть большой блок
    Memory::Free(this->heapType, ptr);
}

//------------------------------------------------------------------------------
/**
*/
const MemoryPool&
PoolArrayAllocator::GetMemoryPool(IndexT index) const
{
    n_assert(index < NumPools);
    return this->memoryPools[index];
}

//------------------------------------------------------------------------------
/**
*/
#if NEBULA_MEMORY_STATS
void
PoolArrayAllocator::Dump()
{
    Util::String msg;
    msg.Format("Дамп PoolArrayAllocator '%s':\n", this->name);
    Core::SysFunc::DebugOut(msg.AsCharPtr());
    bool allOk = true;
    IndexT i;
    for (i = 0; i < NumPools; i++)
    {
        uint allocCount = this->memoryPools[i].GetAllocCount();
        if (0 != allocCount)
        {
            msg.Format("*** %d УТЕЧКИ ДЛЯ РАЗМЕРА БЛОКА %d!!!\n", allocCount, this->memoryPools[i].GetBlockSize());
            Core::SysFunc::DebugOut(msg.AsCharPtr());
            allOk = false;
        }
    }
    if (allOk)
    {
        Core::SysFunc::DebugOut("ВСЕ ОК!\n");
    }
}
#endif
} // пространство имен Память
