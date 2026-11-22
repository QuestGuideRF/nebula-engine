//-------------------------------------------------------------------------------
//  linuxthread.cc
//  (C) 2010 Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//-------------------------------------------------------------------------------
#include "core/config.h"
#include "foundation/stdneb.h"
#include "linuxthread.h"

#if !(__OSX__ || __NACL__)
#include <sched.h>
#include <sys/prctl.h>
#endif

#if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
#include "util/localstringatomtable.h"
#endif

#if __ANDROID__
#include "nvidia/nv_thread/nv_thread.h"
#endif

namespace Linux
{
__ImplementClass(Linux::LinuxThread, 'THRD', Core::RefCounted);

using namespace System;
using namespace Util;

#if NEBULA_DEBUG
Threading::CriticalSection LinuxThread::criticalSection;
List<LinuxThread*> LinuxThread::ThreadList;
#endif

//------------------------------------------------------------------------------
/**
*/
LinuxThread::LinuxThread() :
    priority(Normal),
    stackSize(0),
    threadState(Initial),
    thread(0)
{
    CPU_ZERO(&this->affinity);
    // зарегистрироваться со списком тем
    #if NEBULA_DEBUG
    LinuxThread::criticalSection.Enter();
    this->threadListIterator = ThreadList.AddBack(this);
    LinuxThread::criticalSection.Leave();
    #endif
}

//------------------------------------------------------------------------------
/**
*/
LinuxThread::~LinuxThread()
{
    if (this->IsRunning())
    {
        this->Stop();
    }

    // отменить регистрацию из списка тем
    #if NEBULA_DEBUG
    n_assert(nullptr != *this->threadListIterator);
    LinuxThread::criticalSection.Enter();
    ThreadList.Remove(this->threadListIterator);
    LinuxThread::criticalSection.Leave();
    this->threadListIterator = nullptr;
    #endif
}

//------------------------------------------------------------------------------
/**
*/
void
LinuxThread::Start()
{
    n_assert(!this->IsRunning());

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // установить приоритет
    #if !__NACL__
    /*
    NOTE: don't mess around with thread priorities (at least for now)

    struct sched_param schedParam;
    pthread_attr_getschedparam(&attr, &schedParam);
    switch (this->priority)
    {
        case Low: schedParam.sched_priority = 24; break;
        case Normal: break;
        case High: schedParam.sched_priority = 16; break;
    }
    pthread_attr_setschedparam(&attr, &schedParam);
    */
    #endif
    // не связывайтесь с размером стека pthreads, если вы не знаете, что делаете.
    if(this->stackSize != 0)
    {
        pthread_attr_setstacksize(&attr, this->stackSize);
    }

    // начать ветку
    #if __ANDROID__
        // на Android собственные потоки должны быть зарегистрированы в среде выполнения Java
        NVThreadSpawnJNIThread(&this->thread, &attr, ThreadProc, (void*)this);
    #else
        pthread_create(&this->thread, &attr, ThreadProc, (void*)this);
    #endif
    pthread_attr_destroy(&attr);

    // ИСПРАВЛЕНИЕ: в NACL используйте pthread_setschedprio(), чтобы установить
    // приоритет потока (-> каковы допустимые значения приоритета?)

    // ИСПРАВЛЕНИЕ: размер стека потоков не установлен?

    // if affinity is set apply it
    if(CPU_COUNT(&this->affinity))
    {
        pthread_setaffinity_np(this->thread, sizeof(cpu_set_t), &this->affinity);        
    }
    // дождаться запуска потока
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
LinuxThread::EmitWakeupSignal()
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
LinuxThread::Stop()
{
    if(this->IsRunning())
    {
        // сигнализировать потоку об остановке
        this->stopRequestEvent.Signal();

        // вызвать метод пробуждения-потока, может быть получен в подклассе
        // if the threads needs to be woken up, it is important that this
        // метод вызывается ПОСЛЕ сигнала stopRequestEvent!
        this->EmitWakeupSignal();
        // дождаться завершения потока
    }
    pthread_join(this->thread, 0);
    this->threadState = Stopped;
}

//------------------------------------------------------------------------------
/**
    Internal static helper method. This is called by pthread_create() and
    simply calls the virtual DoWork() method on the thread object.
*/
void *
LinuxThread::ThreadProc(void *self)
{
    n_assert(0 != self);
    n_dbgout("LinuxThread::ThreadProc(): поток запущен!\n");

    #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
    // настроить таблицу атомов локальных строк потока (будет отброшена при завершении потока)
    LocalStringAtomTable* localStringAtomTable = n_new(LocalStringAtomTable);
    #endif

    LinuxThread* threadObj = static_cast<LinuxThread*>(self);
    LinuxThread::SetMyThreadName(threadObj->GetName());
    threadObj->threadState = Running;
    threadObj->threadStartedEvent.Signal();
    threadObj->DoWork();
    threadObj->threadState = Stopped;
    // отказаться от локальной таблицы атомов строк
    #if NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES
    n_delete(localStringAtomTable);
    #endif
    // сообщить системе памяти, что поток завершается
    // FIXME, в настоящее время не реализован и не используется.
    // Память::OnExitThread();

    return 0;
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если поток в данный момент выполняется.
*/
bool
LinuxThread::IsRunning() const
{
    return (Running == this->threadState);
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
LinuxThread::DoWork()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Статический метод, который возвращает ThreadId этого потока.
*/
Threading::ThreadId
LinuxThread::GetMyThreadId()
{
    return (pthread_t) pthread_self();
}

//------------------------------------------------------------------------------
/**
    Статический метод, который возвращает запрошенное состояние этого объекта.
    нить. Еще не реализовано в Linux (см. улучшения в thread-local-
    система данных под Linux в мобильной ветке Nebula).
*/
bool
LinuxThread::GetMyThreadStopRequested()
{
    // ИСПРАВИТЬ! подробности см. в реализации Windows
    return false;
}

//------------------------------------------------------------------------------
/**
    Откажитесь от отрезка времени.
*/
void
LinuxThread::YieldThread()
{
    sched_yield();
}

//------------------------------------------------------------------------------
/**
    Возвращает массив с информацией обо всех существующих на данный момент объектах потока.
*/
#if NEBULA_DEBUG
Array<LinuxThread::ThreadDebugInfo>
LinuxThread::GetRunningThreadDebugInfos()
{
    // ПРИМЕЧАНИЕ. Части этого цикла не полностью потокобезопасны.
    // (например, получение имени потока), но поскольку эти
    // атрибуты не изменяются при запуске потока
    // это не должно быть проблемой.
    Array<ThreadDebugInfo> infos;
    LinuxThread::criticalSection.Enter();
    List<LinuxThread*>::Iterator iter;
    for (iter = ThreadList.Begin(); iter != ThreadList.End(); iter++)
    {
        LinuxThread* cur = *iter;
        if (cur->IsRunning())
        {
            ThreadDebugInfo info;
            info.threadName = cur->GetName();
            info.threadPriority = cur->GetPriority();
            info.threadCoreId = (System::Cpu::CoreId)cur->GetThreadAffinity();
            info.threadStackSize = cur->GetStackSize();
            infos.Append(info);
        }
    }
    LinuxThread::criticalSection.Leave();
    return infos;
}
#endif

//------------------------------------------------------------------------------
/**
 */
void
LinuxThread::SetMyThreadName(const String& n)
{
    #if __OSX__
    // OSX представляет собой UNIX в стиле BSD и имеет функции pthread_np.
    pthread_setname_np(n.AsCharPtr());
    #elif __NACL__
    // FIXME: невозможно установить имя потока в NACL?
    #else
    // Linux немного сложнее...
    String cappedName = n;
    if (cappedName.Length() > 15)
    {
        cappedName.TerminateAtIndex(15);
    }
    prctl(PR_SET_NAME, (unsigned long) cappedName.AsCharPtr(), 0, 0, 0);
    #endif
}

//------------------------------------------------------------------------------
/**
 */
const char *
LinuxThread::GetMyThreadName()
{
    #if __OSX__
    // OSX представляет собой UNIX в стиле BSD и имеет функции pthread_np.
    return "<FIXME>";   
    #else
    static char buffer[16];
    prctl(PR_GET_NAME, buffer,0,0,0,0);
    return buffer;
    #endif
}

//------------------------------------------------------------------------------
/**
 */
int
LinuxThread::GetMyThreadPriority()
{
    sched_param param;
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &param);
    return param.sched_priority;
}

//------------------------------------------------------------------------------
/**
 */
void
LinuxThread::SetThreadAffinity(uint mask)
{    
    CPU_SET(mask, &this->affinity);
    if(this->thread != 0)
    {
        pthread_setaffinity_np(this->thread, sizeof(cpu_set_t), &this->affinity);
    }
}

//------------------------------------------------------------------------------
/**
 */
uint
LinuxThread::GetThreadAffinity()
{    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if(this->thread != 0)
    {
        pthread_getaffinity_np(this->thread, sizeof(cpu_set_t), &cpuset);
    }
    if(CPU_COUNT(&cpuset) == 1)
    {
        return (uint)cpuset.__bits[0];
    }
    return 0;
}

} // пространство имен Linux
