//------------------------------------------------------------------------------
//  osxheap.cc
//  (C) 2010 Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "memory/osx/osxheap.h"
#include "core/sysfunc.h"

namespace OSX
{

#if NEBULA_MEMORY_STATS
List<OSXHeap*>* OSXHeap::list = 0;
CriticalSection* OSXHeap::criticalSection = 0;
#endif

//------------------------------------------------------------------------------
/**
    Этот метод необходимо вызывать в начале приложения, прежде чем
    порождаются любые потоки.
 */
void
OSXHeap::Setup()
{
    #if NEBULA_MEMORY_STATS
    n_assert(0 == list);
    n_assert(0 == criticalSection);
    list = n_new(List<OSXHeap*>);
    criticalSection = n_new(CriticalSection);
    #endif
}
    
//------------------------------------------------------------------------------
/**
*/
OSXHeap::OSXHeap(const char* heapName, size_t initialSize)
{
    n_assert(0 != heapName);
    this->name = heapName;
    this->heapZone = malloc_create_zone(initialSize, 0);
    
    #if NEBULA_MEMORY_STATS
    n_assert(0 != criticalSection);
    this->allocCount = 0;
    this->allocSize = 0;
    criticalSection->Enter();
    this->listIterator = list->AddBack(this);
    criticalSection->Leave();
    #endif
}

//------------------------------------------------------------------------------
/**
*/
OSXHeap::~OSXHeap()
{
    #if NEBULA_MEMORY_STATS
    this->DumpLeaks();
    #endif
    
    malloc_destroy_zone(this->heapZone);
    this->heapZone = 0;
    
    // Дамп утечек памяти и отсоединение от списка кучи
    #if NEBULA_MEMORY_STATS
    n_assert(0 == this->allocCount);
    n_assert(0 != criticalSection);
    n_assert(0 != this->listIterator);
    criticalSection->Enter();
    list->Remove(this->listIterator);
    criticalSection->Leave();
    this->listIterator = 0;
    #endif   
}

//------------------------------------------------------------------------------
/**
*/
void*
OSXHeap::Alloc(size_t size, size_t alignment)
{
    void* allocPtr = malloc_zone_memalign(this->heapZone, alignment, size);
    if (0 == allocPtr)
    {
        n_error("OSXHeap::Alloc(): в куче «%s» недостаточно памяти, попытка выделить %i байт!\n", this->name, int(size));
    }
    #if NEBULA_MEMORY_STATS
    size_t allocatedSize = malloc_size(allocPtr);
    Threading::Interlocked::Increment(this->allocCount);
    Threading::Interlocked::Add(this->allocSize, allocatedSize);
    #endif
    return allocPtr;
}
        
//------------------------------------------------------------------------------
/**
*/
void*
OSXHeap::Realloc(void* ptr, size_t size)
{
    #if NEBULA_MEMORY_STATS
    size_t oldSize = malloc_size(ptr);
    #endif
    void* allocPtr = malloc_zone_realloc(this->heapZone, ptr, size);
    if (0 == allocPtr)
    {
        n_error("OSXHeap::Realloc(): недостаточно памяти в куче «%s», попытка перераспределить %i байт!\n", this->name, (int)size);
    }
    #if NEBULA_MEMORY_STATS
    size_t newSize = malloc_size(ptr);
    Threading::Interlocked::Add(this->allocSize, int(newSize - oldSize));
    #endif
    return allocPtr;
}

//------------------------------------------------------------------------------
/**
*/
void
OSXHeap::Free(void* ptr)
{
    n_assert(0 != ptr);
    #if NEBULA_MEMORY_STATS
    size_t allocSize = malloc_size(ptr);
    Threading::Interlocked::Add(this->allocSize, -int(allocSize));
    Threading::Interlocked::Decrement(this->allocCount);
    #endif
    malloc_zone_free(this->heapZone, ptr);
}

#if NEBULA_MEMORY_STATS
//------------------------------------------------------------------------------
/**
    Подтвердите кучу. Это проходит по блоку памяти кучи и проверяет
    структуры управления. Если что-то не так, функция будет
    остановите программу, иначе функции вернут true.
*/
bool
OSXHeap::ValidateHeap() const
{
    return malloc_zone_check(this->heapZone);
}

//------------------------------------------------------------------------------
/**
*/
int
OSXHeap::GetAllocCount() const
{
    return this->allocCount;
}
    
//------------------------------------------------------------------------------
/**
*/
int
OSXHeap::GetAllocSize() const
{
    return this->allocSize;
}
    
//------------------------------------------------------------------------------
/**
*/
Array<OSXHeap::Stats>
OSXHeap::GetAllHeapStats()
{
    n_assert(0 != criticalSection);
    Array<Stats> result;
    criticalSection->Enter();
    List<OSXHeap*>::Iterator iter;
    for (iter = list->Begin(); iter != list->End(); iter++)
    {
        Stats stats;
        stats.name = (*iter)->GetName();
        stats.allocCount = (*iter)->GetAllocCount();
        stats.allocSize = (*iter)->GetAllocSize();
        result.Append(stats);
    }
    criticalSection->Leave();
    return result
}
        
//------------------------------------------------------------------------------
/**
    Этот статический метод вызывает метод ValidateHeap() для всех куч.
*/
bool
OSXHeap::ValidateAllHeaps()
{
    n_assert(0 != criticalSection);
    criticalSection->Enter();
    bool result = true;
    List<OSXHeap*>::Iterator iter;
    for (iter = list->Begin(); iter != list->End(); iter++)
    {
        result &= (*iter)->ValidateHeap();
    }
    criticalSection->Leave();
    return result;
}
    
//------------------------------------------------------------------------------
/**
    Создает отчет об утечке памяти для этой кучи.
*/
void
OSXHeap::DumpLeaks()
{
    Core::SysFunc::Error("OSXHeap::DumpLeaks() не реализован!\n");
}

//------------------------------------------------------------------------------
/**
    Генерирует утечки памяти для каждого объекта кучи.
*/
void
OSXHeap::DumpLeaksAllHeaps()
{
    n_assert(0 != criticalSection);
    criticalSection->Enter();
    List<OSXHeap*>::Iterator iter;
    for (iter = list->Begin(); iter != list->End(); iter++)
    {
        (*iter)->DumpLeaks();
    }
    criticalSection->Leave();
}
#endif // NEBULA_MEMORY_STATS

} // пространство имен Память