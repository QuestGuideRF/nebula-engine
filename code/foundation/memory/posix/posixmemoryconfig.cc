//------------------------------------------------------------------------------
//  posixmemoryconfig.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "memory/posix/posixmemoryconfig.h"
#include "core/sysfunc.h"

namespace Memory
{
void* volatile Heaps[NumHeapTypes] = { NULL };

//------------------------------------------------------------------------------
/**
*/
void
SetupHeaps()
{
    const size_t megaByte = 1024 * 1024;
    unsigned int i;
    for (i = 0; i < NumHeapTypes; i++)
    {
        n_assert(0 == Heaps[i]);
        size_t initialSize = 0;
        bool useLowFragHeap = false;
        switch (i)
        {
            case DefaultHeap:
            case ObjectHeap:
            case ObjectArrayHeap:
                // эти кучи начинаются с 8 мегабайт и могут увеличиваться
                initialSize = 4 * megaByte;
                useLowFragHeap = true;
                break;

            case ResourceHeap:
            case PhysicsHeap:
            case AppHeap:
            case NetworkHeap:
            case ScriptingHeap:
                // большие кучи блоков и ресурсов начинаются с 32 мегабайт и могут расти(?)
                initialSize = 8 * megaByte;
                break;

            case ScratchHeap:
                // the scratch heap is set to 8 megabyte and may not grow
                initialSize = 8 * megaByte;
                break;

            case StringDataHeap:
                // куча строк начинается с 2 МБ и может увеличиваться
                initialSize = 2 * megaByte;
                useLowFragHeap = true;
                break;

            case StreamDataHeap:
                // куча данных потока начинается с 16 МБ и может увеличиваться
                initialSize = 16 * megaByte;
                break;

            default:
                Core::SysFunc::Error("Неверный тип кучи в Memory::SetupHeaps() (posixmemoryconfig.cc)!");
                break;
        }
        if (0 != initialSize)
        {
            Heaps[i] = NULL; // XXX: HeapCreate (HEAP_GENERATE_EXCEPTIONS, начальный размер, максимальный размер);
            if (useLowFragHeap)
            {
                #if __WIN32__
                // включить Win32 LowFragmentationHeap
                ULONG heapFragValue = 2;
                HeapSetInformation(Heaps[i], HeapCompatibilityInformation, &heapFragValue, sizeof(heapFragValue));
                #endif
            }
        }
        else
        {
            Heaps[i] = 0;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
const char*
GetHeapTypeName(HeapType heapType)
{
    switch (heapType)
    {
        case DefaultHeap:               return "Куча по умолчанию";
        case ObjectHeap:                return "Куча объектов";
        case ObjectArrayHeap:           return "Куча массива объектов";
        case ResourceHeap:              return "Куча ресурсов";
        case ScratchHeap:               return "Царапинная куча";
        case StringDataHeap:            return "Куча строковых данных";
        case StreamDataHeap:            return "Куча потоковых данных";
        case PhysicsHeap:               return "Физика Куча";
        case AppHeap:                   return "Куча приложений";
        case NetworkHeap:               return "Сетевая куча";
        case ScriptingHeap:             return "Куча сценариев";
        default:
            Core::SysFunc::Error("Неверный аргумент HeapType в Memory::GetHeapTypeName()! (win32memoryconfig.cc)");
            return 0;
    }
}

} // пространство имен Память
