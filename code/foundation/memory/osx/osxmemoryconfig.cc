//------------------------------------------------------------------------------
//  osxmemoryconfig.cc
//  (C) 2010 Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "memory/osx/osxmemoryconfig.h"
#include "core/sysfunc.h"

namespace Memory
{
malloc_zone_t* Heaps[NumHeapTypes];

//------------------------------------------------------------------------------
/**
    Настройте глобальные кучи.
*/
void
SetupHeaps()
{
    const size_t megaByte = 1024 * 1024;
    unsigned int i;
    for (i = 0; i < NumHeapTypes; i++)
    {
        size_t startSize = 0;
        switch (i)
        {
            case DefaultHeap:       startSize = 2 * megaByte; break;
            case ObjectHeap:        startSize = 32 * megaByte; break;
            case ObjectArrayHeap:   startSize = 64 * megaByte; break;
            case ResourceHeap:      startSize = 8 * megaByte; break;
            case ScratchHeap:       startSize = 4 * megaByte; break;
            case StringDataHeap:    startSize = 1 * megaByte; break;
            case StreamDataHeap:    startSize = 32 * megaByte; break;
            case PhysicsHeap:       startSize = 1 * megaByte; break;
            case AppHeap:           startSize = 1 * megaByte; break;
                
            default:
                Core::SysFunc::Error("Memory::SetupHeaps(): неверный тип кучи!\n");
                break;
        }
        
        Heaps[i] = malloc_create_zone(startSize, 0);
        n_assert(0 != Heaps[i]);
    }
}
    
//------------------------------------------------------------------------------
/**
*/
const char*
GetHeapTypeName(HeapType heapType)
{
    switch(heapType)
    {
        case DefaultHeap:       return "Куча по умолчанию";
        case ObjectHeap:        return "Куча объектов";
        case ObjectArrayHeap:   return "Куча массива объектов";
        case ResourceHeap:              return "Куча ресурсов";
        case ScratchHeap:               return "Царапинная куча";
        case StringDataHeap:            return "Куча строковых данных";
        case StreamDataHeap:            return "Куча потоковых данных";
        case PhysicsHeap:               return "Физика Куча";
        case AppHeap:                   return "Куча приложений";
        default:
            Core::SysFunc::Error("Неверный аргумент HeapType в Memory::GetHeapTypeName()!");
            return 0;
    }
}
    
} // пространство имен OSX