//------------------------------------------------------------------------------
//  win360thread.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "threading/win32/win32thread.h"

namespace Win32
{
__ImplementClass(Win32::Win32Thread, 'THRD', Core::RefCounted);

using namespace Util;
using namespace System;

ThreadLocal const char* Win32Thread::ThreadName = 0;

#if NEBULA_DEBUG
Threading::CriticalSection Win32Thread::criticalSection;
List<Win32Thread*> Win32Thread::ThreadList;
#endif

//------------------------------------------------------------------------------
/**
*/
Win32Thread::Win32Thread() :
    threadHandle(0),
    affinityMask(0),
    priority(Normal),
    stackSize(NEBULA_THREAD_DEFAULTSTACKSIZE)
{
    // зарегистрироваться со списком тем
    #if NEBULA_DEBUG
        Win32Thread::criticalSection.Enter();
        this->threadListIterator = ThreadList.AddBack(this);
        Win32Thread::criticalSection.Leave();
    #endif
}

//------------------------------------------------------------------------------
/**
*/
Win32Thread::~Win32Thread()
{
    if (this->IsRunning())
    {
        this->Stop();
    }

    // отменить регистрацию из списка тем
    #if NEBULA_DEBUG
        n_assert(this->threadListIterator != nullptr);
        Win32Thread::criticalSection.Enter();
        ThreadList.Remove(this->threadListIterator);
        Win32Thread::criticalSection.Leave();
        this->threadListIterator = 0;
    #endif
}

//------------------------------------------------------------------------------
/**
    Start the thread, this creates a Win32 thread and calls the static
    ThreadProc, which in turn calls the virtual DoWork() class of this object.
    The method waits for the thread to start and then returns.
*/
void
Win32Thread::Start()
{
    n_assert(!this->IsRunning());
    n_assert(0 == this->threadHandle);
    this->threadHandle = CreateThread(NULL,             // lpThreadAttributes
                                      this->stackSize,  // dwStackSize
                                      ThreadProc,       // lpStartAddress
                                      (LPVOID) this,    // lpParameter
                                      CREATE_SUSPENDED, // dwCreationFlags
                                      NULL);            // lpThreadId
    n_assert(NULL != this->threadHandle);

    // применить приоритет потока
    switch (this->priority)
    {
        case Low:   
            SetThreadPriority(this->threadHandle, THREAD_PRIORITY_BELOW_NORMAL);
            break;

        case Normal:
            SetThreadPriority(this->threadHandle, THREAD_PRIORITY_NORMAL);
            break;

        case High:
            SetThreadPriority(this->threadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
            break;
    }
    
    if (this->affinityMask != 0)
        SetThreadAffinityMask(this->threadHandle, this->affinityMask);

    // выберите хороший процессор для темы
    /*
    SystemInfo systemInfo;
    SizeT numCpuCores = systemInfo.GetNumCpuCores();
    DWORD threadIdealProc = 0;
    if (Cpu::InvalidCoreId != this->coreId)
    {
        threadIdealProc = this->coreId % systemInfo.GetNumCpuCores();
    }
    SetThreadIdealProcessor(this->threadHandle, threadIdealProc);
    */

    // возобновить тред (поскольку он был активен в приостановленном состоянии)
    ResumeThread(this->threadHandle);

    // дождитесь начала треда
    this->threadStartedEvent.Wait();
}

//------------------------------------------------------------------------------
/**
    This method is called by Thread::Stop() after setting the 
    stopRequest event and before waiting for the thread to stop. If your
    thread runs a loop and waits for jobs it may need an extra wakeup
    signal to stop waiting and check for the ThreadStopRequested() event. In
    this case, override this method and signal your event object.
*/
void
Win32Thread::EmitWakeupSignal()
{
    // пусто, переопределить в подклассе!
}

//------------------------------------------------------------------------------
/**
    This stops the thread by signalling the stopRequestEvent and waits for the
    thread to actually quit. If the thread code runs in a loop it should use the 
    IsStopRequested() method to see if the thread object wants it to shutdown. 
    If so DoWork() should simply return.
*/
void
Win32Thread::Stop()
{
    n_assert(this->IsRunning());
    n_assert(0 != this->threadHandle);

    // сигнализировать потоку об остановке
    this->stopRequestEvent.Signal();

    // вызвать метод пробуждения-потока, может быть получен в подклассе
    // if the threads needs to be woken up, it is important that this
    // метод вызывается ПОСЛЕ сигнала stopRequestEvent!
    this->EmitWakeupSignal();

    // дождаться завершения потока
    WaitForSingleObject(this->threadHandle, INFINITE);
    CloseHandle(this->threadHandle);
    this->threadHandle = 0;
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если поток в данный момент выполняется.
*/
bool
Win32Thread::IsRunning() const
{
    if (0 != this->threadHandle)
    {
        DWORD exitCode = 0;
        if (GetExitCodeThread(this->threadHandle, &exitCode))
        {
            if (STILL_ACTIVE == exitCode)
            {
                return true;
            }
        }
    }
    // провал: потока еще нет или уже нет
    return false;
}

//------------------------------------------------------------------------------
/**
    This method should be derived in a Thread subclass and contains the
    actual code which is run in the thread. The method must not call
    C-Lib functions under Win32. To terminate the thread, just return from
    this function. If DoWork() runs in an infinite loop, call ThreadStopRequested()
    to check whether the Thread object wants the thread code to quit.
*/
void
Win32Thread::DoWork()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Internal static helper method. This is called by CreateThread() and
    simply calls the virtual DoWork() method on the thread object.
*/
DWORD WINAPI
Win32Thread::ThreadProc(LPVOID self)
{
    n_assert(0 != self);
    
    #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
    // настроить таблицу атомов локальных строк потока (будет отброшена при завершении потока)
    LocalStringAtomTable localStringAtomTable;
    #endif

    Win32Thread* threadObj = (Win32Thread*) self;
    Win32Thread::SetMyThreadName(threadObj->GetName().AsCharPtr());
    threadObj->threadStartedEvent.Signal();
    threadObj->DoWork();

    return 0;
}

//------------------------------------------------------------------------------
/**
    Статический метод, который задает имя этого потока. Это называется от
    внутри ThreadProc. Строка, на которую указывает, должна оставаться действительной до тех пор, пока
    ветка прекращена!
*/
void
Win32Thread::SetMyThreadName(const char* n)
{
    // сначала обновите наш собственный внутренний указатель имени потока
    ThreadName = n;

    // обновите имя потока Windows, чтобы оно отображалось правильно
    // в отладчике
    struct THREADNAME_INFO
    {
        DWORD dwType;     // должно быть 0x1000
        LPCSTR szName;    // указатель на имя (в адресном пространстве пользователя)
        DWORD dwThreadID; // идентификатор потока (-1 = поток вызывающего абонента)
        DWORD dwFlags;    // зарезервировано для будущего использования, должно быть равно нулю
    };

    THREADNAME_INFO info;
    info.dwType     = 0x1000;
    info.szName     = n;
    info.dwThreadID = ::GetCurrentThreadId();
    info.dwFlags    = 0;
    __try
    {
        RaiseException( 0x406D1388, 0, sizeof(info) / sizeof(SIZE_T), (SIZE_T*)&info );
    }
    __except( EXCEPTION_CONTINUE_EXECUTION ) 
    {
    }
}

//------------------------------------------------------------------------------
/**
    Статический метод для получения текущего имени потока из любого места
    в коде потока.
*/
const char*
Win32Thread::GetMyThreadName()
{
    return ThreadName;
}

//------------------------------------------------------------------------------
/**
    Статический метод, который возвращает ThreadId этого потока.
*/
Threading::ThreadId
Win32Thread::GetMyThreadId()
{
    return ::GetCurrentThreadId();
}

//------------------------------------------------------------------------------
/**
    Функция доходности пуста в Win32 и Xbox360.
*/
void
Win32Thread::YieldThread()
{
    BOOL r = SwitchToThread();
    //n_assert (р);
}

//------------------------------------------------------------------------------
/**
    Возвращает массив с информацией обо всех существующих на данный момент объектах потока.
*/
#if NEBULA_DEBUG
Array<Win32Thread::ThreadDebugInfo>
Win32Thread::GetRunningThreadDebugInfos()
{
    // ПРИМЕЧАНИЕ. Части этого цикла не полностью потокобезопасны.
    // (например, получение имени потока), но поскольку эти
    // атрибуты не изменяются при запуске потока
    // это не должно быть проблемой.
    Array<ThreadDebugInfo> infos;
    Win32Thread::criticalSection.Enter();
    List<Win32Thread*>::Iterator iter;
    for (iter = ThreadList.Begin(); iter != ThreadList.End(); iter++)
    {
        Win32Thread* cur = *iter;
        if (cur->IsRunning())
        {
            ThreadDebugInfo info;
            info.threadName = cur->GetName();
            info.threadPriority = cur->GetPriority();
            info.threadStackSize = cur->GetStackSize();
            info.threadCoreId = (System::Cpu::CoreId)cur->GetThreadAffinity();
            infos.Append(info);
        }
    }
    Win32Thread::criticalSection.Leave();
    return infos;
}

#endif

} // пространство имен Win32
