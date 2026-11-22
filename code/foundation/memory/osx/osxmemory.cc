//------------------------------------------------------------------------------
//  osxmemory.cc
//  (C) 2010 РадонЛабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "core/types.h"
#include "core/sysfunc.h"

// #include "threading/interlocked.h"

// переопределить операторы создания/удаления
void* operator new(size_t size)
{
    return Memory::Alloc(Memory::ObjectHeap, size);
}
void* operator new(size_t size, size_t align)
{
    return Memory::Alloc(Memory::ObjectHeap, size, align);
}
void* operator new[](size_t size)
{
    return Memory::Alloc(Memory::ObjectArrayHeap, size);
}
void* operator new[](size_t size, size_t align)
{
    return Memory::Alloc(Memory::ObjectArrayHeap, size, align);
}
void operator delete(void* p)
{
    Memory::Free(Memory::ObjectHeap, p);
}
void operator delete[](void* p)
{
    Memory::Free(Memory::ObjectArrayHeap, p);
}

namespace Memory
{
    
#if NEBULA_MEMORY_STATS
int volatile TotalAllocCount = 0;
int volatile TotalAllocSize = 0;
int volatile HeapTypeAllocCount[NumHeapTypes] = { 0 };
int volatile HeapTypeAllocSize[NumHeapTypes] = { 0 };
bool volatile MemoryLoggingEnabled = false;
unsigned int volatile MemoryLoggingThreshold = 0;
HeapType volatile MemoryLoggingHeapType = InvalidHeapType;
#endif
    
//------------------------------------------------------------------------------
/**
    Выделите блок памяти из одной из глобальных куч.
*/
void*
Alloc(HeapType heapType, size_t size, size_t alignment)
{
    n_assert(heapType < NumHeapTypes);
    
    // убедитесь, что все уже настроено
    Core::SysFunc::Setup();
        
    void* allocPtr = 0;
#if NEBULA_MEMORY_STATS
    size_t allocatedSize = 0;
#endif
        
    // выделить память из глобальной кучи    
    allocPtr = malloc_zone_memalign(Heaps[heapType], alignment, size);
    if (0 == allocPtr)
    {
        n_error("Не удалось выполнить выделение из кучи '%s'!\n", GetHeapTypeName(heapType));
    }
#if NEBULA_MEMORY_STATS
    allocatedSize = malloc_size(allocPtr);
#endif
        
#if NEBULA_MEMORY_STATS                
    Threading::Interlocked::Increment(TotalAllocCount);
    Threading::Interlocked::Add(TotalAllocSize, allocatedSize);
    Threading::Interlocked::Increment(HeapTypeAllocCount[heapType]);
    Threading::Interlocked::Add(HeapTypeAllocSize[heapType], allocatedSize);
    if (MemoryLoggingEnabled && (size >= MemoryLoggingThreshold) &&
        ((MemoryLoggingHeapType == InvalidHeapType) || (MemoryLoggingHeapType == heapType)))
    {
        n_printf("Выделить(размер=%d, allocSize=%d, heapType=%d): 0x%lx\n", size, allocatedSize, heapType, (long unsigned int) allocPtr);
    }
#endif
    return allocPtr;
}
    
//------------------------------------------------------------------------------
/**
    Перераспределить блок памяти из одной из глобальных куч.
 
    ОБРАТИТЕ ВНИМАНИЕ, что эту функцию также можно использовать для сжатия блока памяти!
*/
void*
Realloc(HeapType heapType, void* ptr, size_t size)
{
    n_assert(heapType < NumHeapTypes);
    
    // убедитесь, что все уже настроено
    Core::SysFunc::Setup();
                
    // получить старый размер для отслеживания статистики
#if NEBULA_MEMORY_STATS
    size_t oldSize = malloc_size(ptr);
#endif
        
    // перераспределить память
    void* allocPtr = malloc_zone_realloc(Heaps[heapType], ptr, size);
    if (0 == allocPtr)
    {
        n_error("Не удалось выполнить выделение из кучи '%s'!\n", GetHeapTypeName(heapType));
    }
        
#if NEBULA_MEMORY_STATS
    size_t allocatedSize = mspace_malloc_usable_size(allocPtr);
    Threading::Interlocked::Add(TotalAllocSize, size_t(allocatedSize - oldSize));
    Threading::Interlocked::Add(HeapTypeAllocSize[heapType], size_t(allocatedSize - oldSize));
    if (MemoryLoggingEnabled && (size >= MemoryLoggingThreshold) &&
        ((MemoryLoggingHeapType == InvalidHeapType) || (MemoryLoggingHeapType == heapType)))
    {
        n_printf("Reallocate(size=%d, allocSize=%d, heapType=%d): 0x%lx\n", size, allocatedSize, heapType, (long unsigned int) allocPtr);
    }
#endif
    return allocPtr;
}
    
//------------------------------------------------------------------------------
/**
    Освободите блок памяти.
*/
void
Free(HeapType heapType, void* ptr)
{
    if (0 != ptr)
    {
        n_assert(heapType < NumHeapTypes);
            
#if NEBULA_MEMORY_STATS
        size_t allocatedSize = malloc_size(ptr);
#endif
        malloc_zone_free(Heaps[heapType], ptr);
            
#if NEBULA_MEMORY_STATS
        Threading::Interlocked::Add(TotalAllocSize, -allocatedSize);
        Threading::Interlocked::Decrement(TotalAllocCount);
        Threading::Interlocked::Add(HeapTypeAllocSize[heapType], -allocatedSize);
        Threading::Interlocked::Decrement(HeapTypeAllocCount[heapType]);
        if (MemoryLoggingEnabled && (allocatedSize >= MemoryLoggingThreshold) &&
            ((MemoryLoggingHeapType == InvalidHeapType) || (MemoryLoggingHeapType == heapType)))
        {
            n_printf("Mem::Free(heapType=%d, ptr=0x%lx, allocSize=%d)\n", heapType, (long unsigned int) ptr, allocatedSize);
        }
#endif
    }
}
    
//------------------------------------------------------------------------------
/**
    Дублируйте строку, завершающуюся нулем, этот метод больше не следует использовать!
*/
char*
DuplicateCString(const char* from)
{
    n_assert(0 != from);
    size_t len = (unsigned int) strlen(from) + 1;
    char* to = (char*) Memory::Alloc(Memory::StringDataHeap, len);
    Memory::Copy((void*)from, to, len);
    return to;
}
    
//------------------------------------------------------------------------------
/**
    Проверьте, перекрываются ли две области памяти.
*/
bool
IsOverlapping(const unsigned char* srcPtr, size_t srcSize, const unsigned char* dstPtr, size_t dstSize)
{
    if (srcPtr == dstPtr)
    {
        return true;
    }
    else if (srcPtr > dstPtr)
    {
        return (srcPtr + srcSize) > dstPtr;
    }
    else
    {
        return (dstPtr + dstSize) > srcPtr;
    }
}
    
//------------------------------------------------------------------------------
/**
    Получите общее состояние памяти системы.
*/
TotalMemoryStatus
GetTotalMemoryStatus()
{
    n_error("РЕАЛИЗУЙТЕ МЕНЯ: GetTotalMemoryStatus()!");
    TotalMemoryStatus status = { 0 };
    return status;
}
    
//------------------------------------------------------------------------------
/**
    Copy a chunk of memory (note the argument order is different from memcpy()!!!)
*/
void
Copy(const void* from, void* to, size_t numBytes)
{
    if (numBytes > 0)
    {
        n_assert(0 != from);
        n_assert(0 != to);
        n_assert(from != to);
        memcpy(to, from, numBytes);
    }
}
    
//------------------------------------------------------------------------------
/**
    Перезаписать часть памяти нулями.
*/
void
Clear(void* ptr, size_t numBytes)
{
    memset(ptr, 0, numBytes);
}
    
//------------------------------------------------------------------------------
/**
    Заполнить память определенным байтом.
*/
void
Fill(void* ptr, size_t numBytes, unsigned char value)
{
    memset(ptr, value, numBytes);
}
    
#if NEBULA_MEMORY_STATS
//------------------------------------------------------------------------------
/**
    Включите ведение журнала памяти.
*/
void
EnableMemoryLogging(unsigned int threshold, HeapType heapType)
{
    MemoryLoggingEnabled = true;
    MemoryLoggingThreshold = threshold;
    MemoryLoggingHeapType = heapType;
}
    
//------------------------------------------------------------------------------
/**
    Отключите запись в память.
*/
void
DisableMemoryLogging()
{
    MemoryLoggingEnabled = false;
}
    
//------------------------------------------------------------------------------
/**
    Переключить журналирование памяти.
*/
void
ToggleMemoryLogging(unsigned int threshold, HeapType heapType)
{
    if (MemoryLoggingEnabled)
    {
        DisableMemoryLogging();
    }
    else
    {
        EnableMemoryLogging(threshold, heapType);
    }
}
#endif // NEBULA_MEMORY_STATS
    
#if NEBULA_MEMORY_ADVANCED_DEBUGGING
//------------------------------------------------------------------------------
/**
    Функция отладки, которая проверяет все кучи. 
*/
bool
ValidateMemory()
{
    n_error("РЕАЛИЗУЙТЕ МЕНЯ: ValidateMemory()");
    return true;
}
    
//------------------------------------------------------------------------------
/**
    Запишите информацию об отладке памяти в журнал.
*/
void
Checkpoint(const char* msg)
{
    n_printf("ЖУРНАЛ ПАМЯТИ: %s\n", msg);
    ValidateMemory();
    
    // также выгрузите общее количество/размер выделения памяти по типу кучи...
    n_printf("СЧЕТ/РАЗМЕР РАСПРЕДЕЛЕНИЯ ТУМАННОСТИ: %d / %d\n", TotalAllocCount, TotalAllocSize);
    IndexT i;
    for (i = 0; i < NumHeapTypes; i++)
    {
        const char* heapName = GetHeapTypeName((HeapType)i);
        if (0 == heapName)
        {
            heapName = "UNKNOWN";
        }
        n_printf("КУЧА %l x ALLOC COUNT/SIZE: %s %d / %d\n", Heaps[i], heapName, HeapTypeAllocCount[i], HeapTypeAllocSize[i]);
    }
}
    
//------------------------------------------------------------------------------
/**
    Создайте дамп утечки памяти для всех куч, управляемых Nebula.
*/
void
DumpMemoryLeaks()
{
    n_error("РЕАЛИЗУЙТЕ МЕНЯ: DumpMemoryLeaks()");
}
#endif // NEBULA_MEMORY_ADVANCED_DEBUGGING
    
} // пространство имен Память
