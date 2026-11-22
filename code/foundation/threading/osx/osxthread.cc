//-------------------------------------------------------------------------------
//  osxthread.cc
//  (C) 2010 Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//-------------------------------------------------------------------------------
#include "stdneb.h"
#include "osxthread.h"

namespace OSX
{
__ImplementClass(OSX::OSXThread, 'THRD', Core::RefCounted);
    
using namespace System;    
    
//------------------------------------------------------------------------------
/**
*/
OSXThread::OSXThread() :
    priority(Normal),
    stackSize(16384),
    coreId(Cpu::InvalidCoreId),
    threadState(Initial)
{
    // зарегистрироваться со списком тем
    #if NEBULA_DEBUG
    OSXThread::criticalSection.Enter();
    this->threadListIterator = ThreadList.AddBack(this);
    OSXThread::criticalSection.Leave();
    #endif
}

//------------------------------------------------------------------------------
/**
*/
OSXThread::~OSXThread()
{
    if (this->IsRunning())
    {
        this->Stop();
    }
    
    // отменить регистрацию из списка тем
    #if NEBULA_DEBUG
    n_assert(0 != this->threadListIterator);
    PS3Thread::criticalSection.Enter();
    ThreadList.Remove(this->threadListIterator);
    PS3Thread::criticalSection.Leave();
    this->threadListIterator = 0;
    #endif
}
    
//------------------------------------------------------------------------------
/**
*/
void
OSXThread::Start()
{
    n_error("OSXThread::Start(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
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
OSXThread::EmitWakeupSignal()
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
OSXThread::Stop()
{
    n_error("OSXThread::Stop(): РЕАЛИЗУЙТЕ МЕНЯ!");
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если поток в данный момент выполняется.
*/
bool
OSXThread::IsRunning() const
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
OSXThread::DoWork()
{
    // empty
}
    
//------------------------------------------------------------------------------
/**
    Статический метод, который возвращает ThreadId этого потока.
*/
Threading::ThreadId
OSXThread::GetMyThreadId()
{
    n_error("OSXThread::GetMyThreadId(): РЕАЛИЗУЙТЕ МЕНЯ!");
    return 0;
}
    
//------------------------------------------------------------------------------
/**
    Откажитесь от отрезка времени.
*/
void
OSXThread::YieldThread()
{
    n_error("OSXThread::YieldThread(): РЕАЛИЗУЙТЕ МЕНЯ!");
}
    
//------------------------------------------------------------------------------
/**
    Возвращает массив с информацией обо всех существующих на данный момент объектах потока.
*/
#if NEBULA_DEBUG
Array<OSXThread::ThreadDebugInfo>
OSXThread::GetRunningThreadDebugInfos()
{
    // ПРИМЕЧАНИЕ. Части этого цикла не полностью потокобезопасны.
    // (например, получение имени потока), но поскольку эти
    // атрибуты не изменяются при запуске потока
    // это не должно быть проблемой.
    Array<ThreadDebugInfo> infos;
    OSXThread::criticalSection.Enter();
    List<OSXThread*>::Iterator iter;
    for (iter = ThreadList.Begin(); iter != ThreadList.End(); iter++)
    {
        OSXThread* cur = *iter;
        if (cur->IsRunning())
        {
            ThreadDebugInfo info;
            info.threadName = cur->GetName();
            info.threadPriority = cur->GetPriority();
            info.threadCoreId = cur->GetCoreId();
            info.threadStackSize = cur->GetStackSize();
            infos.Append(info);
        }
    }
    OSXThread::criticalSection.Leave();
    return infos;
}
#endif
    
} // пространство имен OSX