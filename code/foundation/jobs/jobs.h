#pragma once
#include "ids/id.h"
#include "ids/idallocator.h"
#include "threading/thread.h"
#include "threading/event.h"
#include "threading/safequeue.h"
#include "util/stringatom.h"
#include <atomic>
namespace Jobs
{
#define JOBFUNCCONTEXT_MAXIO 8
#define JOBFUNCCONTEXT_MAXUNIFORMS 4
#define N_JOB_INPUT(ctx, slice, index) (ctx.inputs[index] + slice * ctx.inputSizes[index])
#define N_JOB_OUTPUT(ctx, slice, index) (ctx.outputs[index] + slice * ctx.outputSizes[index])
struct JobFuncContext
{
    ubyte* scratch;
    uint numUniforms;
    ubyte* uniforms[JOBFUNCCONTEXT_MAXUNIFORMS];
    uint uniformSizes[JOBFUNCCONTEXT_MAXUNIFORMS];
    uint numInputs;
    ubyte* inputs[JOBFUNCCONTEXT_MAXIO];
    uint inputSizes[JOBFUNCCONTEXT_MAXIO];
    uint numOutputs;
    ubyte* outputs[JOBFUNCCONTEXT_MAXIO];
    uint outputSizes[JOBFUNCCONTEXT_MAXIO];
    uint numSlices;
};
struct JobIOData
{
    static const SizeT MaxNumBuffers = JOBFUNCCONTEXT_MAXIO;
    SizeT numBuffers;
    void* data[MaxNumBuffers];
    SizeT dataSize[MaxNumBuffers];
    SizeT sliceSize[MaxNumBuffers];
};
struct JobUniformData
{
    static const SizeT MaxNumBuffers = JOBFUNCCONTEXT_MAXUNIFORMS;
    SizeT numBuffers;
    const void* data[MaxNumBuffers];
    SizeT dataSize[MaxNumBuffers];
    SizeT scratchSize;
};
struct JobContext
{
    JobIOData input;
    JobIOData output;
    JobUniformData uniform;
};
class JobThread : public Threading::Thread
{
    __DeclareClass(JobThread);
public:
    enum JobThreadCommandType
    {
        RunJob
        , Signal
        , Wait
        , WaitAndReset
    };
    struct JobThreadCommand
    {
        JobThreadCommandType ev;
        union
        {
            struct
            {
                uint slice;
                uint numSlices;
                JobContext context;
                void(*JobFunc)(const JobFuncContext& ctx);
                const std::function<void()>* callback;
            } run;
            struct
            {
                Threading::Event* ev;
                std::atomic_uint* completionCounter;
                const std::function<void()>* callback;
            } sync;
        };
    };
    JobThread();
    virtual ~JobThread();
    void EmitWakeupSignal();
    void DoWork();
    bool HasWork();
    void RunJobSlices(uint sliceIndex, uint numSlices, const JobContext ctx, void(*JobFunc)(const JobFuncContext& ctx), const std::function<void()>* callback);
    void PushCommand(const JobThreadCommand& command);
    void PushCommands(const Util::Array<JobThreadCommand>& commands);
private:
    static const SizeT MaxScratchSize = (64 * 1024);
    Threading::SafeQueue<JobThreadCommand> commands;
    ubyte* scratchBuffer;
};
ID_32_TYPE(JobId);
ID_16_TYPE(JobPortId);
ID_32_TYPE(JobSyncId);
struct CreateJobPortInfo
{
    Util::StringAtom name;
    SizeT numThreads;
    uint affinity;
    uint priority;
};
JobPortId CreateJobPort(const CreateJobPortInfo& info);
void DestroyJobPort(const JobPortId& id);
bool JobPortBusy(const JobPortId& id);
enum
{
    JobPort_Name,
    JobPort_Threads,
    JobPort_NextThreadIndex,
};
typedef Ids::IdAllocator<
    Util::StringAtom,
    Util::FixedArray<Ptr<JobThread>>,
    uint
> JobPortAllocator;
extern JobPortAllocator jobPortAllocator;
struct CreateJobInfo
{
    void(*JobFunc)(const JobFuncContext& ctx);
};
JobId CreateJob(const CreateJobInfo& info);
void DestroyJob(const JobId& id);
void JobSchedule(const JobId& job, const JobPortId& port, const JobContext& ctx, const bool cycleThreads = true);
void JobSchedule(const JobId& job, const JobPortId& port, const JobContext& ctx, const std::function<void()>& callback, const bool cycleThreads = true);
void JobSchedule(const JobId& job, const JobPortId& port);
void JobSchedule(const JobId& job, const JobPortId& port, void* ctx, SizeT count, SizeT groupSize, const bool cycleThreads = true);
void JobScheduleSequence(const Util::Array<JobId>& jobs, const JobPortId& port, const Util::Array<JobContext>& contexts);
void JobScheduleSequence(const Util::Array<JobId>& jobs, const JobPortId& port, const Util::Array<JobContext>& contexts, const std::function<void()>& callback);
void* JobAllocateScratchMemory(const JobId& job, const Memory::HeapType heap, const SizeT size);
struct PrivateMemory
{
    Memory::HeapType heapType;
    SizeT size;
    void* memory;
};
enum
{
    Job_CreateInfo,
    Job_CallbackFunc,
    Job_ScratchMemory
};
typedef Ids::IdAllocator<
    CreateJobInfo,
    std::function<void()>,
    PrivateMemory
> JobAllocator;
extern JobAllocator jobAllocator;
struct CreateJobSyncInfo
{
    std::function<void()> callback;
};
enum
{
    SyncCallback,
    SyncCompletionEvent,
    SyncCompletionCounter,
    SyncPendingSignal
};
JobSyncId CreateJobSync(const CreateJobSyncInfo& info);
void DestroyJobSync(const JobSyncId id);
void JobSyncHostReset(const JobSyncId id);
void JobSyncHostSignal(const JobSyncId id, bool reset = true);
void JobSyncThreadSignal(const JobSyncId id, const JobPortId port, bool reset = true);
void JobSyncHostWait(const JobSyncId id, bool reset = false);
void JobSyncThreadWait(const JobSyncId id, const JobPortId port, bool reset = false);
bool JobSyncSignaled(const JobSyncId id);
typedef Ids::IdAllocator<
    std::function<void()>,
    Threading::Event*,
    std::atomic_uint*,
    bool
> JobSyncAllocator;
extern JobSyncAllocator jobSyncAllocator;
}