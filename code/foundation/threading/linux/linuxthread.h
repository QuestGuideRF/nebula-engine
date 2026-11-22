#pragma once
#include "core/refcounted.h"
#include "threading/threadid.h"
#include "threading/event.h"
#include "system/cpu.h"
namespace Linux
{
class LinuxThread : public Core::RefCounted
{
    __DeclareClass(LinuxThread);
public:
    enum Priority
    {
        Low,
        Normal,
        High,
    };
    LinuxThread();
    virtual ~LinuxThread();
    void SetPriority(Priority p);
    Priority GetPriority() const;
    void SetThreadAffinity(uint mask);
    uint GetThreadAffinity();
    void SetStackSize(SizeT s);
    SizeT GetStackSize() const;
    void SetMaxStackSize(SizeT s);
    void SetName(const Util::String& n);
    const Util::String& GetName() const;
    void Start();
    void Stop();
    bool IsRunning() const;
    static void YieldThread();
    static const char* GetMyThreadName();
    static Threading::ThreadId GetMyThreadId();
    static bool GetMyThreadStopRequested();
    static int GetMyThreadPriority();
    #if NEBULA_DEBUG
    struct ThreadDebugInfo
    {
        Util::String threadName;
        LinuxThread::Priority threadPriority;
        System::Cpu::CoreId threadCoreId;
        SizeT threadStackSize;
    };
    static Util::Array<ThreadDebugInfo> GetRunningThreadDebugInfos();
    #endif
protected:
    static void *ThreadProc(void *);
    virtual void EmitWakeupSignal();
    virtual void DoWork();
    bool ThreadStopRequested() const;
private:
    static void SetMyThreadName(const Util::String& n);
    enum ThreadState
    {
        Initial,
        Running,
        Stopped,
    };
    pthread_t thread;
    cpu_set_t affinity;
    Priority priority;
    SizeT stackSize;
    Util::String name;
    ThreadState volatile threadState;
    Threading::Event threadStartedEvent;
    Threading::Event stopRequestEvent;
#if NEBULA_DEBUG
    static Threading::CriticalSection criticalSection;
    static Util::List<LinuxThread*> ThreadList;
    Util::List<LinuxThread*>::Iterator threadListIterator;
#endif
};
inline void
LinuxThread::SetPriority(Priority p)
{
    this->priority = p;
}
inline LinuxThread::Priority
LinuxThread::GetPriority() const
{
    return this->priority;
}
inline void
LinuxThread::SetStackSize(SizeT s)
{
}
inline SizeT
LinuxThread::GetStackSize() const
{
    return this->stackSize;
}
inline void
LinuxThread::SetMaxStackSize(SizeT s)
{
    this->stackSize = s;
}
inline bool
LinuxThread::ThreadStopRequested() const
{
    return this->stopRequestEvent.Peek();
}
inline void
LinuxThread::SetName(const Util::String& n)
{
    n_assert(n.IsValid());
    this->name = n;
}
inline const Util::String&
LinuxThread::GetName() const
{
    return this->name;
}
}