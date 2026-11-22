#pragma once
#ifndef POSIX_POSIXTHREAD_H
#define POSIX_POSIXTHREAD_H
#include "core/refcounted.h"
#include "threading/posix/posixevent.h"
#include "threading/threadid.h"
#include "system/cpu.h"
#include <sched.h>
namespace Posix
{
class PosixThread : public Core::RefCounted
{
    __DeclareClass(PosixThread);
public:
    enum Priority
    {
        Low,
        Normal,
        High,
    };
    PosixThread();
    virtual ~PosixThread();
    void SetPriority(Priority p);
    Priority GetPriority() const;
    void SetThreadAffinity(uint mask);
    uint GetThreadAffinity();
    void SetStackSize(unsigned int s);
    unsigned int GetStackSize() const;
    void SetName(const Util::String& n);
    const Util::String& GetName() const;
    void Start();
    void Stop();
    bool IsRunning() const;
    static void YieldThread();
    static const char* GetMyThreadName();
    static Threading::ThreadId GetMyThreadId();
protected:
    virtual void EmitWakeupSignal();
    virtual void DoWork();
    bool ThreadStopRequested();
private:
    static void* ThreadProc(void* self);
    pthread_t threadHandle;
    cpu_set_t affinity;
    PosixEvent stopRequestEvent;
    bool running;
    Priority priority;
    unsigned int stackSize;
    Util::String name;
    static ThreadLocal const char* ThreadName;
};
inline bool
PosixThread::IsRunning() const
{
    return (0 != this->threadHandle);
}
inline void
PosixThread::SetPriority(Priority p)
{
    this->priority = p;
}
inline PosixThread::Priority
PosixThread::GetPriority() const
{
    return this->priority;
}
inline bool
PosixThread::ThreadStopRequested()
{
    return this->stopRequestEvent.Peek();
}
inline void
PosixThread::SetName(const Util::String& n)
{
    n_assert(n.IsValid());
    this->name = n;
}
inline const Util::String&
PosixThread::GetName() const
{
    return this->name;
}
}
#endif