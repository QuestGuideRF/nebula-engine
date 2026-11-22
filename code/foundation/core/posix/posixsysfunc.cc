//------------------------------------------------------------------------------
//  sysfunc.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "core/posix/posixsysfunc.h"
#include "core/refcounted.h"
#include "debug/minidump.h"
#include "util/blob.h"
#include "util/guid.h"
#include "net/socket/socket.h"
#include "debug/minidump.h"
#include "threading/thread.h"
#include "util/globalstringatomtable.h"
#include "util/localstringatomtable.h"  
#include "system/systeminfo.h"
#include <errno.h>

namespace Posix
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
        //Threading::Thread::SetMyThreadName("MainThread");
        #endif
        Memory::SetupHeaps();
        Memory::Heap::Setup();
        Blob::Setup();
        #if !__MAYA__
        Net::Socket::InitNetwork();
        #endif   

        globalStringAtomTable = new Util::GlobalStringAtomTable;
        #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
            localStringAtomTable = new Util::LocalStringAtomTable;
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
    // удалить таблицы строковых атомов
    delete globalStringAtomTable;
    #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
        delete localStringAtomTable;
    #endif
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

    // вызвать методы статического завершения работы
    Blob::Shutdown();

    // выключение глобального объекта фабрики
    Core::Factory::Destroy();

    // сообщить об утечке памяти
    #if NEBULA_MEMORY_ADVANCED_DEBUGGING
    Memory::DumpMemoryLeaks();
    #endif   

    // наконец завершить процесс
    exit(exitCode);
}

//------------------------------------------------------------------------------
/**
*/
void
SysFunc::Error(const char* error)
{                
    fprintf(stderr, error);
    abort();
}

//------------------------------------------------------------------------------
/**
*/
void
Posix::SysFunc::MessageBox(const char* msg)
{
    // ничего не делайте, кроме как написать сообщение в консоль
    fprintf(stdout, msg);
}

//------------------------------------------------------------------------------
/**
    Спите определенное количество секунд, откажитесь от отрезка времени.
*/
void
SysFunc::Sleep(double sec)
{
    int milliSecs = int(sec * 1000000.0);
    usleep(milliSecs);
}

//------------------------------------------------------------------------------
/**
    Поместите сообщение в консоль отладки.
*/
void
SysFunc::DebugOut(const char* msg)
{
    fprintf(stderr, msg);
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
} // пространство имен Posix
