#pragma once
#include "threading/thread.h"
#include "threading/event.h"
#include "util/stringatom.h"
#include "threading/interlocked.h"
namespace Threading
{
class Event;
}
namespace Jobs2
{
#define JOB_SIGNATURE (SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
#define JOB_BEGIN_LOOP \
for (IndexT i = 0; i < groupSize; i++)\
{\
    IndexT __INDEX = i + invocationOffset;\
    if (__INDEX >= totalJobs)\
        return;\
#define JOB_END_LOOP \
}
#define JOB_ITEM_INDEX __INDEX
class JobThread;
void* JobAlloc(SizeT bytes);
typedef volatile long CompletionCounter;
using JobFunc = void(*)(SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset, void* ctx);
template <typename... ARGS>
struct CallableStub
{
    virtual void invoke(ARGS... args) = 0;
};
template<typename LAMBDA, typename... ARGS>
struct Callable : CallableStub<ARGS...>
{
    LAMBDA l;
    Callable(LAMBDA l) : l(std::move(l)) {};
    void invoke(ARGS... args) override
    {
        l(args...);
    }
};
struct Lambda
{
    CallableStub<SizeT, SizeT, IndexT, SizeT>* callable;
    template<typename LAMBDA>
    Lambda(LAMBDA l)
    {
        using CallableType = Callable<LAMBDA, SizeT, SizeT, IndexT, SizeT>;
        this->callable = (CallableType*)Jobs2::JobAlloc(sizeof(CallableType));
        new (this->callable) CallableType(l);
    };
    void operator()(SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
    {
        callable->invoke(totalJobs, groupSize, groupIndex, invocationOffset);
    }
};
struct JobContext
{
    JobFunc func;
    Lambda l;
    int remainingGroups;
    Threading::AtomicCounter groupCompletionCounter;
    SizeT numInvocations;
    SizeT groupSize;
    void* data;
    const Threading::AtomicCounter** waitCounters;
    SizeT numWaitCounters;
    Threading::AtomicCounter* doneCounter;
    Threading::Event* signalEvent;
};
struct JobNode
{
    JobNode* next;
    JobContext job;
    JobNode* sequence;
};
struct Jobs2Context
{
    Threading::CriticalSection jobLock;
    JobNode* head = nullptr;
    JobNode* tail = nullptr;
    Util::FixedArray<Ptr<JobThread>> threads;
    Util::Array<JobNode*> queuedJobs;
    SizeT numBuffers;
    IndexT iterator;
    IndexT activeBuffer;
    Util::FixedArray<byte*> scratchMemory;
    SizeT scratchMemorySize;
};
extern Jobs2Context ctx;
class JobThread : public Threading::Thread
{
    __DeclareClass(JobThread);
public:
    JobThread();
    virtual ~JobThread();
    void SignalWorkAvailable();
    bool enableIo;
    bool enableProfiling;
protected:
    virtual void EmitWakeupSignal() override;
    virtual void DoWork() override;
private:
    Threading::Event wakeupEvent;
};
struct JobSystemInitInfo
{
    Util::StringAtom name;
    SizeT numThreads;
    uint affinity;
    uint priority;
    SizeT scratchMemorySize;
    SizeT numBuffers;
    bool enableIo;
    bool enableProfiling;
    JobSystemInitInfo()
        : numThreads(1)
        , affinity(0xFFFFFFFF)
        , priority(UINT_MAX)
        , scratchMemorySize(1_MB)
        , numBuffers(1)
        , enableIo(false)
        , enableProfiling(true)
    {};
};
void JobSystemInit(const JobSystemInitInfo& info);
void JobSystemUninit();
template <typename T> T* JobAlloc(SizeT count);
void* JobAlloc(SizeT bytes);
void JobNewFrame();
extern JobNode* sequenceNode;
extern JobNode* sequenceTail;
extern const Threading::AtomicCounter* prevDoneCounter;
extern Threading::ThreadId sequenceThread;
void JobBeginSequence(const Util::FixedArray<const Threading::AtomicCounter*, true>& waitCounters = nullptr
    , Threading::AtomicCounter* doneCounter = nullptr
    , Threading::Event* signalEvent = nullptr);
template <typename CTX> void JobAppendSequence(
    const JobFunc& func
    , const SizeT numInvocations
    , const SizeT groupSize
    , const CTX& context
);
template <typename CTX> void JobAppendSequence(
    const JobFunc& func
    , const SizeT numInvocations
    , const CTX& context
);
void JobEndSequence(Threading::Event* signalEvent = nullptr);
template <typename T> T*
JobAlloc(SizeT count)
{
    return (T*)JobAlloc(count * sizeof(T));
}
template <typename LAMBDA> void
JobDispatch(
    LAMBDA&& func
    , const SizeT numInvocations
    , const SizeT groupSize
    , const Util::FixedArray<const Threading::AtomicCounter*, true>& waitCounters = nullptr
    , Threading::AtomicCounter* doneCounter = nullptr
    , Threading::Event* signalEvent = nullptr
)
{
    n_assert(numInvocations > 0);
    n_assert(doneCounter != nullptr ? *doneCounter > 0 : true);
    SizeT numJobs = Math::ceil(numInvocations / float(groupSize));
    SizeT dynamicAllocSize = sizeof(JobNode) + waitCounters.Size() * sizeof(const Threading::AtomicCounter*);
    auto mem = JobAlloc<char>(dynamicAllocSize);
    auto node = (JobNode*)mem;
    node->job.waitCounters = nullptr;
    if (waitCounters.Size() > 0)
    {
        node->job.waitCounters = (const Threading::AtomicCounter**)(mem + sizeof(JobNode));
        memcpy(node->job.waitCounters, waitCounters.Begin(), waitCounters.Size() * sizeof(const Threading::AtomicCounter*));
    }
    node->job.l = std::move(func);
    node->job.func = nullptr;
    node->job.remainingGroups = numJobs;
    node->job.groupCompletionCounter = numJobs;
    node->job.numInvocations = numInvocations;
    node->job.groupSize = groupSize;
    node->job.numWaitCounters = (SizeT)waitCounters.Size();
    node->job.doneCounter = doneCounter;
    node->job.signalEvent = signalEvent;
    node->sequence = nullptr;
    ctx.jobLock.Enter();
    if (ctx.head == nullptr)
        ctx.head = node;
    node->next = nullptr;
    if (ctx.tail != nullptr)
        ctx.tail->next = node;
    ctx.tail = node;
    ctx.jobLock.Leave();
    for (Ptr<JobThread>& thread : ctx.threads)
    {
        thread->SignalWorkAvailable();
    }
}
template <typename LAMBDA> void
JobDispatch(
    LAMBDA&& func
    , const SizeT numInvocations
    , const Util::FixedArray<const Threading::AtomicCounter*, true>& waitCounters = nullptr
    , Threading::AtomicCounter* doneCounter = nullptr
    , Threading::Event* signalEvent = nullptr
)
{
    JobDispatch(func, numInvocations, numInvocations, waitCounters, doneCounter, signalEvent);
}
template <typename CTX> void
JobDispatch(
    const JobFunc& func
    , const SizeT numInvocations
    , const SizeT groupSize
    , const CTX& context
    , const Util::FixedArray<const Threading::AtomicCounter*, true>& waitCounters = nullptr
    , Threading::AtomicCounter* doneCounter = nullptr
    , Threading::Event* signalEvent = nullptr
)
{
    static_assert(std::is_trivially_destructible<CTX>::value, "Контекст задания должен быть тривиально разрушаемым.");
    n_assert(numInvocations > 0);
    n_assert(doneCounter != nullptr ? *doneCounter > 0 : true);
    SizeT numJobs = Math::ceil(numInvocations / float(groupSize));
    auto dynamicAllocSize = sizeof(JobNode) + sizeof(CTX) + waitCounters.Size() * sizeof(const Threading::AtomicCounter*);
    auto mem = JobAlloc<char>(dynamicAllocSize);
    auto node = (JobNode*)mem;
    node->job.waitCounters = nullptr;
    if (waitCounters.Size() > 0)
    {
        node->job.waitCounters = (const Threading::AtomicCounter**)(mem + sizeof(JobNode) + sizeof(CTX));
        memcpy(node->job.waitCounters, waitCounters.Begin(), waitCounters.Size() * sizeof(const Threading::AtomicCounter*));
    }
    node->job.data = (void*)(mem + sizeof(JobNode));
    auto data = reinterpret_cast<CTX*>(node->job.data);
    *data = context;
    node->job.l.callable = nullptr;
    node->job.func = func;
    node->job.remainingGroups = numJobs;
    node->job.groupCompletionCounter = numJobs;
    node->job.numInvocations = numInvocations;
    node->job.groupSize = groupSize;
    node->job.numWaitCounters = (SizeT)waitCounters.Size();
    node->job.doneCounter = doneCounter;
    node->job.signalEvent = signalEvent;
    node->sequence = nullptr;
    ctx.jobLock.Enter();
    if (ctx.head == nullptr)
        ctx.head = node;
    node->next = nullptr;
    if (ctx.tail != nullptr)
        ctx.tail->next = node;
    ctx.tail = node;
    ctx.jobLock.Leave();
    for (Ptr<JobThread>& thread : ctx.threads)
    {
        thread->SignalWorkAvailable();
    }
}
template <typename CTX> void
JobDispatch(
    const JobFunc& func
    , const SizeT numInvocations
    , const CTX& context
    , const Util::FixedArray<const Threading::AtomicCounter*, true>& waitCounters = nullptr
    , Threading::AtomicCounter* doneCounter = nullptr
    , Threading::Event* signalEvent = nullptr
)
{
    JobDispatch(func, numInvocations, numInvocations, context, waitCounters, doneCounter, signalEvent);
}
template<typename CTX> void
JobAppendSequence(const JobFunc& func, const SizeT numInvocations, const SizeT groupSize, const CTX& context)
{
    static_assert(std::is_trivially_destructible<CTX>::value, "Контекст задания должен быть тривиально разрушаемым.");
    n_assert(numInvocations > 0);
    n_assert(sequenceThread == Threading::Thread::GetMyThreadId());
    n_assert(sequenceNode != nullptr);
    SizeT numJobs = Math::ceil(numInvocations / float(groupSize));
    SizeT dynamicAllocSize = sizeof(JobNode) + sizeof(CTX) + sizeof(Threading::AtomicCounter);
    if (prevDoneCounter != nullptr)
        dynamicAllocSize += sizeof(Threading::AtomicCounter*);
    auto mem = JobAlloc<char>(dynamicAllocSize);
    auto node = (JobNode*)mem;
    if (prevDoneCounter != nullptr)
    {
        node->job.numWaitCounters = 1;
        node->job.waitCounters = (const Threading::AtomicCounter**)(mem + sizeof(JobNode) + sizeof(CTX) + sizeof(Threading::AtomicCounter));
        memcpy(node->job.waitCounters, &prevDoneCounter, sizeof(Threading::AtomicCounter*));
    }
    else
    {
        node->job.numWaitCounters = 0;
        node->job.waitCounters = nullptr;
    }
    node->job.data = (void*)(mem + sizeof(JobNode));
    auto data = reinterpret_cast<CTX*>(node->job.data);
    *data = context;
    node->job.l.callable = nullptr;
    node->job.func = func;
    node->job.remainingGroups = numJobs;
    node->job.groupCompletionCounter = numJobs;
    node->job.numInvocations = numInvocations;
    node->job.groupSize = groupSize;
    node->job.doneCounter = (Threading::AtomicCounter*)(mem + sizeof(JobNode) + sizeof(CTX));
    *node->job.doneCounter = 1;
    prevDoneCounter = node->job.doneCounter;
    node->job.signalEvent = nullptr;
    node->next = nullptr;
    if (sequenceTail == nullptr)
        sequenceNode->sequence = node;
    else
        sequenceTail->next = node;
    sequenceTail = node;
}
template<typename CTX> void
JobAppendSequence(const JobFunc& func, const SizeT numInvocations, const CTX& context)
{
    JobAppendSequence(func, numInvocations, numInvocations, context);
}
}