#pragma once
#include "core/refcounted.h"
#include "core/ptr.h"
#include "distributedtools/remotejobservice.h"
namespace IO
{
    class TextWriter;
}
namespace DistributedTools
{
class DistributedJob;
class DistributedJobScheduler{
public:
    DistributedJobScheduler();
    virtual bool Open();
    virtual void Close();
    bool IsOpen();
    void UseRemoteServices(bool flag);
    void SetMaxParallelLocalJobs(int count);
    virtual void AttachRemoteJobService(const Ptr<RemoteJobService> & service);
    virtual void AttachJob(const Ptr<DistributedJob> & job);
    virtual void RunJobs();
    virtual void Update();
    bool HasActiveJobs();
    void AppendInitializeJob(const Ptr<DistributedJob> & job);
    Util::Array<Ptr<DistributedJob>> GetInitializeJobList();
    void AppendFinalizeJob(const Ptr<DistributedJob> & job);
    Util::Array<Ptr<DistributedJob>> GetFinalizeJobList();
    void PrintLocalJobOutput(bool enable);
private:
    void StartReadyJobs();
    void StartReadyJobsLocal();
    void StartReadyJobsRemote();
    void ProcessLocalJob(const Ptr<DistributedJob> & job);
    bool HasRunningRemoteServices();
    void DisableRunningService(const Ptr<RemoteJobService> & service);
    void HandleBannedService(const Ptr<RemoteJobService> & service);
    void ResetJobsFromService(const Ptr<RemoteJobService> & service);
    Util::Array<Ptr<DistributedJob>> readyJobs;
    Util::Array<Ptr<DistributedJob>> localRunningJobs;
    Util::Array<Ptr<RemoteJobService>> remoteServices;
    Util::Array<Ptr<RemoteJobService>> bannedRemoteServices;
    Util::Array<Ptr<DistributedJob>> remoteRunningJobs;
    Util::Array<Ptr<DistributedJob>> initializeJobs;
    Util::Array<Ptr<DistributedJob>> finalizeJobs;
    Util::Array<Ptr<DistributedJob>> remoteInitializeJobs;
    Util::Array<Ptr<DistributedJob>> remoteFinalizeJobs;
    bool isRunningJobs;
    bool isOpen;
    int maxParallelLocalJob;
    bool useRemoteServices;
    bool printLocalJobOutput;
};
inline
bool
DistributedJobScheduler::IsOpen()
{
    return this->isOpen;
}
inline
void
DistributedJobScheduler::UseRemoteServices(bool flag)
{
    this->useRemoteServices = flag;
}
inline
void
DistributedJobScheduler::SetMaxParallelLocalJobs(int count)
{
    this->maxParallelLocalJob = count;
}
inline
void
DistributedJobScheduler::AppendInitializeJob(const Ptr<DistributedJob> & job)
{
    this->initializeJobs.Append(job);
}
inline
Util::Array<Ptr<DistributedJob>>
DistributedJobScheduler::GetInitializeJobList()
{
    return this->initializeJobs;
}
inline
void
DistributedJobScheduler::AppendFinalizeJob(const Ptr<DistributedJob> & job)
{
    this->finalizeJobs.Append(job);
}
inline
Util::Array<Ptr<DistributedJob>>
DistributedJobScheduler::GetFinalizeJobList()
{
    return this->finalizeJobs;
}
inline
void
DistributedJobScheduler::PrintLocalJobOutput(bool enable)
{
    this->printLocalJobOutput = enable;
}
}