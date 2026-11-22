#pragma once
#include "fiber.h"
#include "ids/id.h"
#include "ids/idallocator.h"
#include "threading/thread.h"
#include "threading/safequeue.h"
#include "threading/lockfreequeue.h"
namespace Fibers
{
struct FiberContext
{
    Fibers::Fiber* fiber;
    volatile int* counter;
};
struct FiberWaitContext
{
    Fibers::Fiber* fiber;
    volatile int* counter;
    int value;
};
class FiberThread : public Threading::Thread
{
    __DeclareClass(FiberThread);
public:
    FiberThread();
    virtual ~FiberThread();
    void EmitWakeupSignal();
    void DoWork();
    bool HasWork();
    void SleepFiber(volatile int* counter, int value);
    void NewFiber();
    void SwitchFiber();
private:
    Fibers::FiberContext currentFiber;
    Fibers::Fiber threadFiber;
};
thread_local extern FiberThread* currentThread;
struct FiberQueueCreateInfo
{
    uint numThreads;
    uint numFibers;
};
class FiberQueue
{
public:
    typedef void (*JobFunction)(void* context);
    typedef void (*FiberFunction)();
    struct Job
    {
        JobFunction function;
        uint id;
        void* context;
        Threading::AtomicCounter* counter;
    };
    FiberQueue();
    virtual ~FiberQueue();
    static void Setup(const FiberQueueCreateInfo& info);
    static void Discard();
    template <class T> static void Enqueue(JobFunction function, const Util::FixedArray<T*>& contexts, Threading::AtomicCounter* counter);
    static void Free(uint id);
    static void Sleep(FiberWaitContext context);
    static bool WakeupFiber(Fibers::FiberContext& fiber);
    static bool Dequeue(Fibers::FiberContext& fiber);
private:
    static Threading::LockFreeQueue<Job> PendingJobsQueue;
    static Threading::LockFreeQueue<uint> FiberIdQueue;
    static Util::FixedArray<Fibers::Fiber> Fibers;
    static Util::FixedArray<FiberQueue::Job> FiberContexts;
    static Util::FixedArray<Ptr<FiberThread>> Threads;
    static Threading::CriticalSection SleepLock;
    static Util::Array<FiberWaitContext> SleepingFibers;
};
template <class T>
inline void
FiberQueue::Enqueue(JobFunction function, const Util::FixedArray<T*>& contexts, Threading::AtomicCounter* counter)
{
    Threading::Interlocked::Exchange(counter, contexts.Size());
    for (uint i = 0; i < contexts.Size(); i++)
    {
        Job job;
        job.function = function;
        job.context = contexts[i];
        job.counter = counter;
        FiberQueue::PendingJobsQueue.Enqueue(job);
    }
}
template <class T>
inline void
Enqueue(FiberQueue::JobFunction function, const Util::FixedArray<T*>& contexts, Threading::AtomicCounter* counter)
{
    FiberQueue::Enqueue(function, contexts, counter);
}
inline void
Wait(Threading::AtomicCounter* counter, int value)
{
    if (*counter != value)
    {
        currentThread->SleepFiber(counter, value);
    }
}
inline void
Lock(Threading::AtomicCounter* counter, int value)
{
    while (*counter != value) {};
}
}