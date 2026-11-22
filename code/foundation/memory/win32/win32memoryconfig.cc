//------------------------------------------------------------------------------
//  win32memoryconfig.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "memory/win32/win32memoryconfig.h"
#include "core/sysfunc.h"

namespace Memory
{
HANDLE volatile Heaps[NumHeapTypes] = { NULL };

//------------------------------------------------------------------------------
/**
    This method is called once at application startup from 
    Core::SysFunc::Setup() to setup the various Nebula heaps.
*/
void
SetupHeaps()
{
    // настроить глобальные кучи
    const SIZE_T megaByte = 1024 * 1024;
    const SIZE_T kiloByte = 1024;
    unsigned int i;
    for (i = 0; i < NumHeapTypes; i++)
    {
        n_assert(0 == Heaps[i]);
        SIZE_T initialSize = 0;
        SIZE_T maxSize = 0;
        bool useLowFragHeap = false;
        switch (i)
        {
            case DefaultHeap:
            case ObjectHeap:
                initialSize = 4 * megaByte;
                useLowFragHeap = true;
                break;

            case ObjectArrayHeap:
                initialSize = 4 * megaByte;
                break;

            case ResourceHeap:
            case PhysicsHeap:
            case AppHeap:
            case NetworkHeap:
            case ScriptingHeap:
                initialSize = 8 * megaByte;
                break;

            case ScratchHeap:
                initialSize = 8 * megaByte;
                break;

            case StringDataHeap:
                initialSize = 2 * megaByte;
                useLowFragHeap = true;
                break;

            case StreamDataHeap:
                initialSize = 16 * megaByte;
                break;

            default:
                Core::SysFunc::Error("Неверный тип кучи в Memory::SetupHeaps() (win32memoryconfig.cc)!");
                break;
        }
        if (0 != initialSize)
        {
            Heaps[i] = HeapCreate(0, initialSize, maxSize);
            if (useLowFragHeap)
            {
                // включить Win32 LowFragmentationHeap
                ULONG heapFragValue = 2;
                HeapSetInformation(Heaps[i], HeapCompatibilityInformation, &heapFragValue, sizeof(heapFragValue));
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

} // пространство имен Win32
