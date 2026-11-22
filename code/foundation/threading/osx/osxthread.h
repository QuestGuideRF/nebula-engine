#pragma once
#include "core/refcounted.h"
#include "threading/threadid.h"
#include "system/cpu.h"
namespace OSX
{
class OSXThread : public Core::RefCounted
{
    __DeclareClass(OSXThread);
public:
    enum Priority
    {
        Low,
        Normal,
        High,
    };
    OSXThread();
    virtual ~OSXThread();
    void SetPriority(Priority p);
    Priority GetPriority() const;
    void SetCoreId(System::Cpu::CoreId coreId);
    System::Cpu::CoreId GetCoreId() const;
    void SetStackSize(SizeT s);
    SizeT GetStackSize() const;
    void SetName(const Util::String& n);
    const Util::String& GetName() const;
    void Start();
    void Stop();
    bool IsRunning() const;
    static void YieldThread();
    static Threading::ThreadId GetMyThreadId();
    #if NEBULA_DEBUG
    #endif
protected:
    virtual void EmitWakeupSignal();
    virtual void DoWork();
    bool ThreadStopRequested() const;
private:
    enum ThreadState
    {
        Initial,
        Running,
        Stopped,
    };
    pthread_t thread;
    Priority priority;
    SizeT stackSize;
    Util::String name;
    System::Cpu::CoreId coreId;
    ThreadState volatile threadState;
#if NEBULA_DEBUG
    static Threading::CriticalSection criticalSection;
    static Util::List<OSXThread*> ThreadList;
    Util::List<OSXThread*>::Iterator threadListIterator;
#endif
};
inline void
OSXThread::SetPriority(Priority p)
{
    this->priority = p;
}
inline OSXThread::Priority
OSXThread::GetPriority() const
{
    return this->priority;
}
inline void
OSXThread::SetStackSize(SizeT s)
{
    this->stackSize = s;
}
inline SizeT
OSXThread::GetStackSize() const
{
    return this->stackSize;
}
inline bool
OSXThread::ThreadStopRequested() const
{
n_error("РЕАЛИЗУЙТЕ МЕНЯ!");
return false;
}
inline void
OSXThread::SetName(const Util::String& n)
{
    n_assert(n.IsValid());
    this->name = n;
}
inline const Util::String&
OSXThread::GetName() const
{
    return this->name;
}
inline void
OSXThread::SetCoreId(System::Cpu::CoreId id)
{
    this->coreId = id;
}
inline System::Cpu::CoreId
OSXThread::GetCoreId() const
{
    return this->coreId;
}
}