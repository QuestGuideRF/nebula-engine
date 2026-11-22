//------------------------------------------------------------------------------
//  posixheap.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "memory/posix/posixheap.h"

namespace Posix
{
using namespace Threading;
using namespace Util;

#if NEBULA_MEMORY_STATS
List<PosixHeap*>* PosixHeap::list = 0;
CriticalSection* PosixHeap::criticalSection = 0;
#endif

//------------------------------------------------------------------------------
/**
    Этот метод необходимо вызывать в начале приложения, поскольку
    создаются любые потоки (обычно вызываемые Util::Setup()).
*/
void
PosixHeap::Setup()
{
    #if NEBULA_MEMORY_STATS
    n_assert(0 == list);
    n_assert(0 == criticalSection);
    list = n_new(List<PosixHeap*>);
    criticalSection = n_new(CriticalSection);
    #endif
}

//------------------------------------------------------------------------------
/**
*/
PosixHeap::PosixHeap(const char* heapName)
{
    n_assert(0 != heapName);
    this->name = heapName;
    
    // ссылка на список кучи
    #if NEBULA_MEMORY_STATS
    n_assert(0 != criticalSection);
    this->allocCount = 0;
    this->allocSize  = 0;
    criticalSection->Enter();
    this->listIterator = list->AddBack(this);
    criticalSection->Leave();
    #endif
}

//------------------------------------------------------------------------------
/**
*/
PosixHeap::~PosixHeap()
{
    // удалить связь со списком кучи
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

#if NEBULA_MEMORY_STATS
//------------------------------------------------------------------------------
/**
    Подтвердите кучу. Это проходит по блоку памяти кучи и проверяет
    структуры управления. Если что-то не так, функция будет
    остановите программу, иначе функции вернут true.
*/
bool
PosixHeap::ValidateHeap() const
{
    BOOL result = HeapValidate(this->heap, 0, NULL);
    return (0 != result);
}


//------------------------------------------------------------------------------
/**
*/
int
PosixHeap::GetAllocCount() const
{
    return this->allocCount;
}

//------------------------------------------------------------------------------
/**
*/
int
PosixHeap::GetAllocSize() const
{
    return this->allocSize;
}

//------------------------------------------------------------------------------
/**
*/
Array<PosixHeap::Stats>
PosixHeap::GetAllHeapStats()
{
    n_assert(0 != criticalSection);
    Array<Stats> result;
    criticalSection->Enter();
    List<PosixHeap*>::Iterator iter;
    for (iter = list->Begin(); iter != list->End(); iter++)
    {
        Stats stats;
        stats.name       = (*iter)->GetName();
        stats.allocCount = (*iter)->GetAllocCount();
        stats.allocSize  = (*iter)->GetAllocSize();        
        result.Append(stats);
    }
    criticalSection->Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
    Этот статический метод вызывает метод ValidateHeap() для всех куч.
*/
bool
PosixHeap::ValidateAllHeaps()
{
    n_assert(0 != criticalSection);
    criticalSection->Enter();
    bool result = true;
    List<PosixHeap*>::Iterator iter;
    for (iter = list->Begin(); iter != list->End(); iter++)
    {
        result &= (*iter)->ValidateHeap();
    }
    criticalSection->Leave();
    return result;
}
#endif // NEBULA_MEMORY_STATS

} // пространство имен Память
