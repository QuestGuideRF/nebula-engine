#pragma once
#include "core/refcounted.h"
#include "core/ptr.h"
#include "timing/timer.h"
#include "net/socket/ipaddress.h"
#include "util/guid.h"
namespace IO
{
    class XmlWriter;
    class XmlReader;
}
namespace Net
{
    class MessageClient;
}
namespace DistributedTools
{
class DistributedJob;
class RemoteJobService : public Core::RefCounted
{
__DeclareClass(DistributedTools::RemoteJobService)
public:
    enum ServiceState
    {
        Closed,
        Connecting,
        Ready,
        Reserved,
        ExclusiveReady,
        Busy,
        ServiceError,
    };
    RemoteJobService();
    virtual bool Open();
    virtual void Close();
    virtual bool SendJob(const Ptr<DistributedJob> & job);
    virtual void RunJobs();
    virtual void Update();
    virtual bool IsOpen();
    virtual ServiceState GetServiceState();
    const Util::Array<Util::Guid> & GetSentJobGuids();
    const Util::Array<Util::Guid> & GetFinishedJobGuids();
    const Util::Array<Util::Guid> & GetFailedJobGuids();
    void RemoveFromFailedJobList(Util::Guid guid);
    void SetDirty();
    void SendBannedInfo();
    bool HasCleanedUp();
    void Reset();
    void SetIpAddress(const Net::IpAddress & ip);
    const Net::IpAddress & GetIpAddress();
    SizeT GetProcessorCount();
    Util::String GetErrorMessage();
    void SetInitializeJobs(const Util::Array<Ptr<DistributedJob>> & jobs );
    const Util::Array<Ptr<DistributedJob>> & GetInitializeJobs();
    void SetFinalizeJobs(const Util::Array<Ptr<DistributedJob>> & jobs );
    const Util::Array<Ptr<DistributedJob>> & GetFinalizeJobs();
    const Util::Array<Ptr<DistributedJob>> & GetUncompleteJobs();
    void SetVerboseFlag(bool val);
private:
    virtual void HandleCommandNode(const Ptr<IO::XmlReader> & reader, const Net::IpAddress & address);
    virtual void HandleStateNode(const Ptr<IO::XmlReader> & reader, const Net::IpAddress & address);
    virtual bool SendStateRequest();
    virtual bool SendDetailedStateRequest();
    void WriteClientHeader(const Ptr<IO::XmlWriter> & writer);
    Ptr<Net::MessageClient> tcpclient;
    bool isOpen;
    bool isClean;
    bool verboseFlag;
    ServiceState state;
    Util::Array<Util::Guid> sentJobGuids;
    Util::Array<Util::Guid> addedJobGuids;
    Util::Array<Util::Guid> failedJobGuids;
    Util::Array<Util::Guid> finishedJobGuids;
    Util::Array<Util::Guid> pendingRequestGuids;
    Util::Array<Ptr<DistributedJob>> uncompleteJobs;
    Util::Array<Ptr<DistributedJob>> initializeJobs;
    Util::Array<Ptr<DistributedJob>> finalizeJobs;
    Timing::Timer timer;
    Timing::Timer lastReceivedAnswerTimer;
    Net::IpAddress ipAddress;
    SizeT processorCount;
    Util::String errorMessage;
    Util::Guid clientID;
};
inline
bool
RemoteJobService::IsOpen()
{
    return this->isOpen;
}
inline
RemoteJobService::ServiceState
RemoteJobService::GetServiceState()
{
    return this->state;
}
inline
const Util::Array<Util::Guid> &
RemoteJobService::GetSentJobGuids()
{
    return this->sentJobGuids;
}
inline
const Util::Array<Util::Guid> &
RemoteJobService::GetFinishedJobGuids()
{
    return this->finishedJobGuids;
}
inline
const Util::Array<Util::Guid> &
RemoteJobService::GetFailedJobGuids()
{
    return this->failedJobGuids;
}
inline
void
RemoteJobService::RemoveFromFailedJobList(Util::Guid guid)
{
    IndexT index = this->failedJobGuids.FindIndex(guid);
    if(index != InvalidIndex)
    {
        this->failedJobGuids.EraseIndex(index);
    }
}
inline
void
RemoteJobService::SetIpAddress(const Net::IpAddress & ip)
{
    this->ipAddress = ip;
}
inline
const Net::IpAddress &
RemoteJobService::GetIpAddress()
{
    return this->ipAddress;
}
inline
SizeT
RemoteJobService::GetProcessorCount()
{
    return this->processorCount;
}
inline
Util::String
RemoteJobService::GetErrorMessage()
{
    n_assert(ServiceError == this->state);
    return this->errorMessage;
}
inline
void
RemoteJobService::SetInitializeJobs(const Util::Array<Ptr<DistributedJob>> & jobs )
{
    this->initializeJobs = jobs;
}
inline
const Util::Array<Ptr<DistributedJob>> &
RemoteJobService::GetInitializeJobs()
{
    return this->initializeJobs;
}
inline
void
RemoteJobService::SetFinalizeJobs(const Util::Array<Ptr<DistributedJob>> & jobs )
{
    this->finalizeJobs = jobs;
}
inline
const Util::Array<Ptr<DistributedJob>> &
RemoteJobService::GetFinalizeJobs()
{
    return this->finalizeJobs;
}
inline
const Util::Array<Ptr<DistributedJob>> &
RemoteJobService::GetUncompleteJobs()
{
    return this->uncompleteJobs;
}
inline
void
RemoteJobService::SetVerboseFlag(bool val)
{
    this->verboseFlag = val;
}
inline
void
RemoteJobService::SetDirty()
{
    this->isClean = false;
}
}