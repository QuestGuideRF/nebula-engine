//------------------------------------------------------------------------------
//  sysfunc.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "core/win32/win32sysfunc.h"
#include "core/refcounted.h"
#include "debug/minidump.h"
#include "util/blob.h"
#include "net/socket/socket.h"
#include "debug/minidump.h"
#include "threading/thread.h"
#include "util/globalstringatomtable.h"
#include "util/localstringatomtable.h"  
#include "system/systeminfo.h"
#include "debug/win32/win32stacktrace.h"
#include <io.h>

namespace Win32
{
using namespace Util;

bool volatile SysFunc::SetupCalled = false;
const Core::ExitHandler* SysFunc::ExitHandlers = 0;
System::SystemInfo SysFunc::systemInfo;

Util::GlobalStringAtomTable* globalStringAtomTable = 0;
#if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
    Util::LocalStringAtomTable* localStringAtomTable = 0;
#endif

//------------------------------------------------------------------------------
/**
    Этот метод должен вызываться при запуске приложения, прежде чем какие-либо потоки
    порождаются. Он используется для предварительной настройки статических объектов (т.
    частные кучи различных классов полезности). Это исключает
    необходимость детальной блокировки затронутых классов.
*/
void
SysFunc::Setup()
{
    if (!SetupCalled)
    {
        SetupCalled = true;
        #if !__MAYA__
        Threading::Thread::SetMyThreadName("MainThread");
        #endif
        Memory::SetupHeaps();
        Memory::Heap::Setup();
        Blob::Setup();
        #if !__MAYA__
        Net::Socket::InitNetwork();
        Debug::MiniDump::Setup();
        #endif   

        globalStringAtomTable = new Util::GlobalStringAtomTable;
        #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
            localStringAtomTable = new Util::LocalStringAtomTable;
        #endif    

        // запросить поддержку SIMD
        int CPUInfo[4] = { -1 };
        __cpuid(CPUInfo, 0x00000001);
#if N_USE_AVX
        if ((CPUInfo[2] & (1 << 28)) != (1 << 28))
            n_error("Ошибка: нет поддержки AVX.");
#endif
        if ((CPUInfo[2] & (1 << 20)) != (1 << 20))
            n_error("Ошибка: нет поддержки SSE4.");
        if ((CPUInfo[3] & (1 << 26)) != (1 << 26))
            n_error("Ошибка: нет поддержки SSE2... что за фигня?");

#if N_USE_FMA
        if ((CPUInfo[2] & (1 << 12)) != (1 << 12))
            n_error("Нет поддержки FMA");
#endif

    }
}

//------------------------------------------------------------------------------
/**
    This method is called by Application::Exit(), or otherwise must be
    called right before the end of the programs main() function. The method
    will properly shutdown the Nebula runtime environment, and report 
    refcounting and memory leaks (debug builds only). This method will not
    return.
*/
void
SysFunc::Exit(int exitCode)
{
    // сначала создайте отчет об утечке RefCount
    #if NEBULA_DEBUG
    Core::RefCounted::DumpRefCountingLeaks();
    #endif

    // обработчики выхода вызова
    const Core::ExitHandler* exitHandler = SysFunc::ExitHandlers;
    while (0 != exitHandler)
    {
        exitHandler->OnExit();
        exitHandler = exitHandler->Next();
    }

    // удалить таблицы строковых атомов
    delete globalStringAtomTable;
#if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
    delete localStringAtomTable;
#endif

    // выключите среду выполнения C, это очистит статические объекты, но не закроет 
    // вниз по процессу
    _cexit();

    // вызвать методы статического завершения работы
    Blob::Shutdown();

    // выключение глобального объекта фабрики
    Core::Factory::Destroy();


    // сообщить об утечке памяти
    #if NEBULA_MEMORY_ADVANCED_DEBUGGING
    Memory::DumpMemoryLeaks();
    #endif   

    // наконец завершить процесс
    ExitProcess(exitCode);
}

//------------------------------------------------------------------------------
/**
    При этом отображается окно сообщения об ошибке Win32 и происходит выход из программы.
*/
void
SysFunc::Error(const char* error)
{
    #ifdef _DEBUG
    OutputDebugString(error);   
    #endif
    /*
    HWND hwnd = FindWindow(NEBULA_WINDOW_CLASS, NULL);
    if (hwnd)
    {
        ShowWindow(hwnd, SW_MINIMIZE);
    }
    */
    Util::Array<Util::String> stacktrace = Win32StackTrace::GenerateStackTrace();
    Util::String format;
    // удалите первые 7 записей, поскольку они являются только функциями подтверждения/ошибки, и последние 6, поскольку они запускают Windows. 
    for (int i = 6; i < Math::min(17,stacktrace.Size() - 6); i++)
    {
        format.Append(stacktrace[i]);
        //format.Append("\n");
    }
    format.Format("%s\n\
----- *** СТЕК ВЫЗОВОВ *** -----\n\
%s\
----- *** СТЕК ВЫЗОВОВ *** -----\n\n", error, format.AsCharPtr());

    if(_isatty(_fileno(stdout)))
    {
#ifdef _DEBUG
        if (IsDebuggerPresent())
        {
            OutputDebugString(format.AsCharPtr());
            fprintf(stderr, "%s", format.AsCharPtr());
            n_break();
            exit(1);
        }
        else
#endif
        {
            ::MessageBox(NULL, format.AsCharPtr(), "СИСТЕМНАЯ ОШИБКА ТУМАННОСТИ", MB_OK | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONERROR);
#if !__MAYA__
            Debug::MiniDump::WriteMiniDump();
#endif
            abort();
        }
    }
    else
    {
#ifdef _DEBUG
        OutputDebugString(format.AsCharPtr());
        fprintf(stderr,"%s\n",format.AsCharPtr());        
        if (IsDebuggerPresent())
        { 
            n_break();
        }        
#else
        exit(1);
#endif
    }
    
}

//------------------------------------------------------------------------------
/**
    Это отображает окно сообщения Win32.
*/
void
SysFunc::MessageBox(const char* msg)
{
    #ifdef _DEBUG
    OutputDebugString(msg);
    #endif
    HWND hwnd = FindWindow(NEBULA_WINDOW_CLASS, NULL);
    if (hwnd)
    {
        ShowWindow(hwnd, SW_MINIMIZE);
    }
    ::MessageBox(NULL, msg, "ТУМАННОСТЬ СООБЩЕНИЕ", MB_OK|MB_APPLMODAL|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONINFORMATION);
}

//------------------------------------------------------------------------------
/**
    Спите определенное количество секунд, откажитесь от отрезка времени.
*/
void
SysFunc::Sleep(double sec)
{
    int milliSecs = int(sec * 1000.0);
    ::Sleep(milliSecs);
}

//------------------------------------------------------------------------------
/**
    Поместите сообщение в консоль отладки.
*/
void
SysFunc::DebugOut(const char* msg)
{
    OutputDebugString(msg);
}

//------------------------------------------------------------------------------
/**
    Register a new exit handler. This method is called at startup time
    from the constructor of static exit handler objects. This is the only
    supported way to register exit handlers. The method will return
    a pointer to the next exit handler in the forward linked list 
    (or 0 if this is the first exit handler).
*/
const Core::ExitHandler*
SysFunc::RegisterExitHandler(const Core::ExitHandler* exitHandler)
{
    const Core::ExitHandler* firstHandler = ExitHandlers;
    ExitHandlers = exitHandler;
    return firstHandler;
}

} // пространство имен Win32
