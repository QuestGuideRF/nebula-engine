//------------------------------------------------------------------------------
//  refcounted.cc
//  (C) 2006 г. РадонЛабс ГмбХ
//  (C) 2013–2020 Индивидуальные участники, см. файл АВТОРОВ (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------

#include "core/refcounted.h"

namespace Core
{
__ImplementRootClass(Core::RefCounted, 'REFC');

#if NEBULA_DEBUG
using namespace Util;

Threading::CriticalSection RefCounted::criticalSection;
ThreadLocal bool RefCounted::isInCreate = false;
RefCountedList RefCounted::list;

#endif

//------------------------------------------------------------------------------
/**
*/
RefCounted::~RefCounted()
{
    n_assert(0 == this->refCount);
    #if NEBULA_DEBUG
        n_assert(!this->destroyed);
        n_assert(this->listIterator != nullptr);
        criticalSection.Enter();
        list.Remove(this->listIterator);
        criticalSection.Leave();
        this->listIterator = nullptr;
        this->destroyed = true;
    #endif
}

//------------------------------------------------------------------------------
/**
    Этот метод следует вызывать как самый последний перед приложением. 
    выходит.
*/
void
RefCounted::DumpRefCountingLeaks()
{
    #if NEBULA_DEBUG
    list.DumpLeaks();    
    #endif
}

//------------------------------------------------------------------------------
/**
    Собирайте статистику по классам.
*/
#if NEBULA_DEBUG
Dictionary<String,RefCounted::Stats>
RefCounted::GetOverallStats()
{
    Dictionary<String,Stats> result;
    criticalSection.Enter();
    // перебирать пересчитанный список
    RefCountedList::Iterator iter;
    for (iter = list.Begin(); iter != list.End(); iter++)
    {
        RefCounted* cur = *iter;
        if (!result.Contains(cur->GetClassName()))
        {
            Stats newStats;
            newStats.className   = cur->GetClassName();
            newStats.classFourCC = cur->GetClassFourCC();
            newStats.numObjects  = 1;
            newStats.overallRefCount = cur->GetRefCount();
            newStats.instanceSize = cur->GetRtti()->GetInstanceSize();
            result.Add(cur->GetClassName(), newStats);
        }
        else
        {
            Stats& stats = result[cur->GetClassName()];
            stats.numObjects++;
            stats.overallRefCount += cur->GetRefCount();
        }
    }
    criticalSection.Leave();
    return result;
}
#endif

} // Фонд пространства имен
