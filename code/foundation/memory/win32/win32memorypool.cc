//------------------------------------------------------------------------------
//  win32memorypool.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "memory/win32/win32memorypool.h"
#include "util/string.h"

namespace Win32
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
Win32MemoryPool::Win32MemoryPool() :
    heapType(Memory::InvalidHeapType),
    blockSize(0),
    alignedBlockSize(0),
    poolSize(0),
    numBlocks(0),
    poolStart(0),
    poolEnd(0)
{
    #if NEBULA_MEMORY_STATS
    this->allocCount = 0;
    #endif
    Memory::Clear(&this->listHead, sizeof(this->listHead));
}

//------------------------------------------------------------------------------
/**
*/
Win32MemoryPool::~Win32MemoryPool()
{
    #if NEBULA_MEMORY_STATS
    if (this->allocCount != 0)
    {
        String str;
        str.Format("Win32MemoryPool: утечка памяти %d в пуле '0x%08x'!\n", this->allocCount, (uintptr_t)this);
        Core::SysFunc::DebugOut(str.AsCharPtr());
    }
    #endif

    // отказаться от пула памяти
    Memory::Free(this->heapType, this->poolStart);
    this->poolStart = 0;
    this->poolEnd = 0;
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. Имя должно быть статической строкой!
*/
void
Win32MemoryPool::Setup(Memory::HeapType heapType_, uint blockSize_, uint numBlocks_)
{
    n_assert(0 == this->poolStart);
    n_assert(0 == this->poolEnd);
    this->blockSize = blockSize_;
    this->numBlocks = numBlocks_;
    this->heapType = heapType_;

    #if NEBULA_MEMORY_STATS
    this->allocCount = 0;
    #endif

    // compute block size with sizeof(*) header
    this->alignedBlockSize = ComputeAlignedBlockSize(this->blockSize);
    this->poolSize = (this->numBlocks * this->alignedBlockSize) + BlockAlign;

    // блок памяти пула настройки, каждая запись в блоке имеет заголовок
    // с указателем на следующий блок, фактическое начало блока памяти
    // выровнен по 16 байтам
    this->poolStart = (ubyte*) Memory::Alloc(this->heapType, this->poolSize);
    this->poolEnd = this->poolStart + this->poolSize;
    
    // настроить прямой список свободных блоков
    Memory::Clear(&this->listHead, sizeof(listHead));
    PSLIST_ENTRY listEntry = (PSLIST_ENTRY) (this->poolStart + (BlockAlign - sizeof(SLIST_ENTRY)));
    uint i;
    for (i = 0; i < this->numBlocks; i++)
    {
        InterlockedPushEntrySList(&this->listHead, listEntry);
        listEntry += this->alignedBlockSize / sizeof(SLIST_ENTRY);
    }
}

//------------------------------------------------------------------------------
/**
*/
void*
Win32MemoryPool::Alloc()
{
    #if NEBULA_MEMORY_STATS
    _InterlockedIncrement(&this->allocCount);
    #endif
    // получить следующий бесплатный блок из списка свободных и исправить список свободных
    PSLIST_ENTRY entry = InterlockedPopEntrySList(&this->listHead);
    if (0 == entry)
    {
        return 0;
    }
    void* ptr = (void*) (((ubyte*)entry) + sizeof(SLIST_ENTRY));

    // заполнить шаблоном отладки
    #if NEBULA_DEBUG
    Memory::Fill(ptr, this->blockSize, NewBlockPattern);
    #endif

    return ptr;
}

//------------------------------------------------------------------------------
/**
*/
void
Win32MemoryPool::Free(void* ptr)
{
    #if NEBULA_MEMORY_STATS
    n_assert(this->allocCount > 0);
    _InterlockedDecrement(&this->allocCount);
    #endif
    // получить указатель на заголовок и список свободных исправлений
    PSLIST_ENTRY entry = (PSLIST_ENTRY) (((ubyte*)ptr) - sizeof(SLIST_ENTRY));
    InterlockedPushEntrySList(&this->listHead, entry);

    // заполнить блок free' шаблоном отладки
    #if NEBULA_DEBUG
    Memory::Fill(ptr, this->blockSize, FreeBlockPattern);
    #endif
}

} // пространство имен Win32
