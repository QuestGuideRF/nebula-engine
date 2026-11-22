//------------------------------------------------------------------------------
//  win32memory.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "core/types.h"
#include "core/sysfunc.h"
#include "memory/heap.h"
#include "memory/poolarrayallocator.h"


namespace Memory
{
HANDLE volatile Win32ProcessHeap = 0;
long volatile TotalAllocCount = 0;
long volatile TotalAllocSize = 0;
long volatile HeapTypeAllocCount[NumHeapTypes] = { 0 };
long volatile HeapTypeAllocSize[NumHeapTypes] = { 0 };
bool volatile MemoryLoggingEnabled = false;
unsigned int volatile MemoryLoggingThreshold = 0;
HeapType volatile MemoryLoggingHeapType = InvalidHeapType;

//------------------------------------------------------------------------------
/**
    Выделите блок памяти из одной из глобальных куч.
*/
void*
Alloc(HeapType heapType, size_t size, size_t align)
{
    n_assert(heapType < NumHeapTypes);
    n_assert(align <= 16);
    // нужно убедиться, что все настроено
    Core::SysFunc::Setup();

    void* allocPtr = 0;    
    {
        n_assert(0 != Heaps[heapType]);
        allocPtr =  __HeapAlloc16(Heaps[heapType], 0, size);
        n_assert(((uintptr_t)allocPtr & 15) == 0);
        if (0 == allocPtr)
        {
            n_error("Выделить: недостаточно памяти, выделяется '%s', попытка выделить '%d' байт\n",
                    GetHeapTypeName(heapType), size);
        }
    }
    #if NEBULA_MEMORY_STATS
        Threading::Interlocked::Increment((volatile int*)&TotalAllocCount);
        Threading::Interlocked::Add((volatile int*)&TotalAllocSize, (long)size + 16);
        Threading::Interlocked::Increment((volatile int*)&HeapTypeAllocCount[heapType]);
        Threading::Interlocked::Add((volatile int*)&HeapTypeAllocSize[heapType], (long)size + 16);
        if (MemoryLoggingEnabled && (size >= MemoryLoggingThreshold) &&
            ((MemoryLoggingHeapType == InvalidHeapType) || (MemoryLoggingHeapType == heapType)))
        {
            n_printf("Выделить(размер=%zx, heapType=%d): 0x%llx\n", size, heapType, (uintptr_t) allocPtr);
        }
    #endif
    return allocPtr;
}

//------------------------------------------------------------------------------
/**
    Перераспределить блок памяти.
*/
void*
Realloc(HeapType heapType, void* ptr, size_t size)
{
    n_assert((heapType < NumHeapTypes) && (0 != Heaps[heapType]));
    #if NEBULA_MEMORY_STATS
        SIZE_T oldSize = __HeapSize16(Heaps[heapType], 0, ptr);
    #endif
    void* allocPtr = __HeapReAlloc16(Heaps[heapType], 0, ptr, size);
    n_assert(((uintptr_t)allocPtr & 15) == 0);
    if (0 == allocPtr)
    {
        n_error("Realloc: недостаточно памяти, выделение '%s', попытка выделить байты '%d'\n",
                GetHeapTypeName(heapType), size);
    }
    #if NEBULA_MEMORY_STATS
        SIZE_T newSize = __HeapSize16(Heaps[heapType], 0, allocPtr);
        Threading::Interlocked::Add((volatile int*)&TotalAllocSize, int(newSize - oldSize + 16));
        Threading::Interlocked::Add((volatile int*)&HeapTypeAllocSize[heapType], int(newSize - oldSize + 16));
        if (MemoryLoggingEnabled && (size >= MemoryLoggingThreshold) &&
            ((MemoryLoggingHeapType == InvalidHeapType) || (MemoryLoggingHeapType == heapType)))
        {
            n_printf("Перераспределить (размер=%zx, heapType=%d): 0x%llx\n", size, heapType, (uintptr_t)allocPtr);
        }
    #endif
    return allocPtr;
}

//------------------------------------------------------------------------------
/**
    Освободите часть памяти из кучи процесса.
*/
void
Free(HeapType heapType, void* ptr)
{
    // D3DX на 360 любит вызывать оператор удаления с указателем 0.
    if (0 != ptr)
    {
        n_assert(heapType < NumHeapTypes);
        #if NEBULA_MEMORY_STATS
            SIZE_T size = 0;
        #endif    
        n_assert(0 != Heaps[heapType]);
        #if NEBULA_MEMORY_STATS
            size = __HeapSize16(Heaps[heapType], 0, ptr);
        #endif
        __HeapFree16(Heaps[heapType], 0, ptr);
        #if NEBULA_MEMORY_STATS
            Threading::Interlocked::Add((volatile int*)&TotalAllocSize, -int(size));
            Threading::Interlocked::Decrement((volatile int*)&TotalAllocCount);
            Threading::Interlocked::Add((volatile int*)&HeapTypeAllocSize[heapType], -int(size));
            Threading::Interlocked::Decrement((volatile int*)&HeapTypeAllocCount[heapType]);
            if (MemoryLoggingEnabled && (size >= MemoryLoggingThreshold) &&
                ((MemoryLoggingHeapType == InvalidHeapType) || (MemoryLoggingHeapType == heapType)))
            {
                n_printf("Mem::Free(heapType=%d, ptr=0x%llx, allocSize=%zx)\n", heapType, (uintptr_t)ptr, size);
            }
        #endif
    }
}

//------------------------------------------------------------------------------
/**
    Duplicate a 0-terminated string. The memory will be allocated from
    the StringDataHeap (important when freeing the memory!)
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
    MEMORYSTATUS stats = { NULL };
    GlobalMemoryStatus(&stats);
    TotalMemoryStatus result;
    result.totalPhysical = (unsigned int) stats.dwTotalPhys;
    result.availPhysical = (unsigned int) stats.dwAvailPhys;
    result.totalVirtual  = (unsigned int) stats.dwTotalVirtual;
    result.availVirtual  = (unsigned int) stats.dwAvailVirtual;
    return result;
}

//------------------------------------------------------------------------------
/**
    Дамп подробной информации о состоянии памяти.
*/
void
DumpTotalMemoryStatus()
{
    n_printf("DumpTotalMemoryStatus() еще не реализован в N3.\n");   
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
    Функция отладки, которая проверяет кучу процесса и все локальные кучи. 
    Останавливает программу, если что-то не так. 
*/
bool
ValidateMemory()
{
    bool res = true;

    // проверить глобальные кучи
    IndexT i;
    for (i = 0; i < NumHeapTypes; i++)
    {
        if (0 != Heaps[i])
        {
            res &= (0 != HeapValidate(Heaps[i], 0, NULL));
        }
    }

    // проверить локальные кучи
    res &= Heap::ValidateAllHeaps();

    // проверить кучу процесса
    res &= (TRUE == ::HeapValidate(::GetProcessHeap(), 0, NULL));

    return res;
}

//------------------------------------------------------------------------------
/**
    Запишите информацию об отладке памяти в журнал.
*/
void
Checkpoint(const char* msg)
{
    n_printf("ЖУРНАЛ ПАМЯТИ: %s\n", msg);
   // Проверка памяти();

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

    // сбросить все кучи процессов Windows
    n_printf("КУЧИ ПРОЦЕССОВ WIN32\n");
    HANDLE procHeaps[100];
    DWORD procHeapIndex;
    DWORD numProcHeaps = ::GetProcessHeaps(100, procHeaps);
    n_printf("%d КУЧИ ПРОЦЕССОВ\n", numProcHeaps);
    const HANDLE defaultHeap = ::GetProcessHeap();
    const HANDLE crtHeap = (HANDLE) _get_heap_handle();
    SizeT totalAllocSize = 0;
    SizeT totalAllocCount = 0;
    SizeT totalUnusedSize = 0;
    SizeT totalUnusedCount = 0;
    for (procHeapIndex = 0; procHeapIndex < numProcHeaps; procHeapIndex++)
    {
        // проверить, является ли текущая куча кучей туманности
        const char* heapName = 0;
        IndexT nebHeapIndex;
        for (nebHeapIndex = 0; nebHeapIndex < NumHeapTypes; nebHeapIndex++)
        {
            if (Heaps[nebHeapIndex] == procHeaps[procHeapIndex])
            {
                heapName = GetHeapTypeName((HeapType)nebHeapIndex);
                break;
            }
        }

        // проверить, является ли это CRT или кучей стандартного процесса
        if (0 == heapName)
        {
            if (defaultHeap == procHeaps[procHeapIndex])
            {
                heapName = "КУЧА ПРОЦЕССА";
            }
            else if (crtHeap == procHeaps[procHeapIndex])
            {
                heapName = "ЭЛТ-КУЧА";
            }
            else
            {
                heapName = "НЕИЗВЕСТНАЯ КУЧА";
            }
        }

        // вычислить количество выделений и размер кучи данных
        PROCESS_HEAP_ENTRY walkEntry = { 0 };
        SizeT allocSize = 0;
        SizeT allocCount = 0;
        SizeT unusedCount = 0;
        SizeT unusedSize = 0;
        ::HeapLock(procHeaps[procHeapIndex]);
        while (::HeapWalk(procHeaps[procHeapIndex], &walkEntry))
        {
            if (walkEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
            {
                allocCount++;
                allocSize += walkEntry.cbData;
            }
            else if (walkEntry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE)
            {
                unusedCount++;
                unusedSize += walkEntry.cbData;
            }
        }
        ::HeapUnlock(procHeaps[procHeapIndex]);
        n_printf("    куча в %lx (%s): allocCount(%d) allocSize(%d) unusedCount(%d) unusedSize(%d)\n",
            procHeaps[procHeapIndex], heapName, allocCount, allocSize, unusedCount, unusedSize);
        totalAllocSize += allocSize;
        totalAllocCount += allocCount;
        totalUnusedSize += unusedSize;
        totalUnusedCount += unusedCount;
    }
    n_printf("    ВСЕГО ПРОЦЕССОВЫХ КУЧ: allocCount(%d), allocSize(%d), unusedCount(%d), unusedSize(%d)\n", 
        totalAllocCount, totalAllocSize, totalUnusedCount, totalUnusedSize);
}

//------------------------------------------------------------------------------
/**
    Создайте дамп утечки памяти для всех куч, управляемых Nebula.
*/
void
DumpMemoryLeaks()
{
    // утечки памяти объекта кучи дампа
    Heap::DumpLeaksAllHeaps();

    // сбросить глобальные кучи
    IndexT i;
    for (i = 0; i < NumHeapTypes; i++)
    {
        if (0 != Heaps[i])
        {
            Heap::DumpHeapMemoryLeaks(GetHeapTypeName((HeapType)i), Heaps[i]);
        }
    }
}
#endif

//------------------------------------------------------------------------------
/**
*/
void*
AllocVirtual(size_t size)
{
    void* ret = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
    n_assert(ret != nullptr);
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void
CommitVirtual(void* ptr, size_t size)
{
    auto ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    n_assert(ret != 0);
}

//------------------------------------------------------------------------------
/**
*/
void
DecommitVirtual(void* ptr, size_t size)
{
    auto ret = VirtualFree(ptr, size, MEM_DECOMMIT);
    n_assert(ret != 0);
}

//------------------------------------------------------------------------------
/**
*/
void
FreeVirtual(void* ptr, size_t size)
{
    auto ret = VirtualFree(ptr, 0, MEM_RELEASE);
    n_assert(ret != 0);
}

} // пространство имен Память

//------------------------------------------------------------------------------
/*
    Переопределить операторы создания/удаления.
*/
void*
operator new(size_t size) noexcept(false)
{
    return Memory::Alloc(Memory::ObjectHeap, size);
}

void*
operator new(size_t size, std::align_val_t al) noexcept(false)
{
    return Memory::Alloc(Memory::ObjectHeap, size, (size_t)al);
}

void*
operator new(size_t size, const std::nothrow_t& noThrow) noexcept
{
    return Memory::Alloc(Memory::ObjectHeap, size);
}

void*
operator new[](size_t size) noexcept(false)
{
    return Memory::Alloc(Memory::ObjectArrayHeap, size);
}

void*
operator new[](size_t size, std::align_val_t al) noexcept(false)
{
    return Memory::Alloc(Memory::ObjectArrayHeap, size, (size_t)al);
}

void*
operator new[](size_t size, const std::nothrow_t& noThrow) noexcept
{
    return Memory::Alloc(Memory::ObjectArrayHeap, size);
}

void
operator delete(void* p)
{
    Memory::Free(Memory::ObjectHeap, p);
}

void
operator delete[](void* p)
{
    Memory::Free(Memory::ObjectArrayHeap, p);
}

