#pragma once
#include "core/refcounted.h"
#include "threading/win32/win32event.h"
#include "threading/threadid.h"
#include "system/cpu.h"
#include "util/localstringatomtable.h"
namespace Win32
{
class Win32Thread : public Core::RefCounted
{
    __DeclareClass(Win32Thread);
public:
    enum Priority
    {
        Low,
        Normal,
        High,
    };
    Win32Thread();
    virtual ~Win32Thread();
    void SetPriority(Priority p);
    Priority GetPriority() const;
    void SetStackSize(SizeT s);
    SizeT GetStackSize() const;
    void SetName(const Util::String& n);
    const Util::String& GetName() const;
    void SetThreadAffinity(const uint mask);
    uint GetThreadAffinity();
    void Start();
    void Stop();
    bool IsRunning() const;
    static void YieldThread();
    static void SetMyThreadName(const char* n);
    static const char* GetMyThreadName();
    static Threading::ThreadId GetMyThreadId();
    #if NEBULA_DEBUG
    struct ThreadDebugInfo
    {
        Util::String threadName;
        Win32Thread::Priority threadPriority;
        System::Cpu::CoreId threadCoreId;
        SizeT threadStackSize;
    };
    static Util::Array<ThreadDebugInfo> GetRunningThreadDebugInfos();
    #endif
protected:
    virtual void EmitWakeupSignal();
    virtual void DoWork();
    bool ThreadStopRequested() const;
private:
    static DWORD WINAPI ThreadProc(LPVOID self);
    HANDLE threadHandle;
    Win32Event threadStartedEvent;
    Win32Event stopRequestEvent;
    Priority priority;
    uint affinityMask;
    SizeT stackSize;
    Util::String name;
    ThreadLocal static const char* ThreadName;
    #if NEBULA_DEBUG
    static Threading::CriticalSection criticalSection;
    static Util::List<Win32Thread*> ThreadList;
    Util::List<Win32Thread*>::Iterator threadListIterator;
    #endif
};
inline void
Win32Thread::SetPriority(Priority p)
{
    this->priority = p;
}
inline Win32Thread::Priority
Win32Thread::GetPriority() const
{
    return this->priority;
}
inline void
Win32Thread::SetStackSize(SizeT s)
{
    this->stackSize = s;
}
inline SizeT
Win32Thread::GetStackSize() const
{
    return this->stackSize;
}
inline bool
Win32Thread::ThreadStopRequested() const
{
    return this->stopRequestEvent.Peek();
}
inline void
Win32Thread::SetName(const Util::String& n)
{
    n_assert(n.IsValid());
    this->name = n;
}
inline const Util::String&
Win32Thread::GetName() const
{
    return this->name;
}
inline void
Win32Thread::SetThreadAffinity(const uint mask)
{
    this->affinityMask = mask;
}
inline uint
Win32Thread::GetThreadAffinity()
{
    return this->affinityMask;
}
};