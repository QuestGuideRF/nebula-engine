#pragma once
#include "toolkit-common/toolkitapp.h"
#include "toolkit-common/distributedtools/shareddircontrol.h"
#include "toolkit-common/distributedtools/shareddircreator.h"
#include "toolkit-common/distributedtools/distributedjobs/distributedjob.h"
namespace DistributedTools
{
class DistributedToolkitApp : public ToolkitUtil::ToolkitApp
{
public:
    DistributedToolkitApp();
    virtual ~DistributedToolkitApp();
    virtual bool ParseCmdLineArgs();
    virtual Util::String GetArgumentDescriptionString();
    virtual bool Open();
    virtual void Run();
    virtual void Close();
    const Ptr<SharedDirControl> & GetSharedDirControl();
    bool IsDistributed();
    bool IsSlave();
    bool IsMaster();
protected:
    virtual void CreateAndProcessJobs();
    virtual Util::Array<Util::String> CreateFileList();
    virtual Util::String GetCurrentRevision();
    virtual void WaitForKey();
    virtual void DoWork();
    virtual void OnBeforeRunLocal();
    virtual void OnAfterRunLocal();
    virtual Util::Array<Ptr<DistributedJob>> GenerateInitializeJobList();
    virtual Util::Array<Ptr<DistributedJob>> GenerateFinalizeJobList();
    virtual void OnBeforeRunDistSlave();
    virtual void OnAfterRunDistSlave();
    virtual void OnBeforeRunDistMaster();
    virtual void OnAfterRunDistMaster();
    Util::String projectdirArg;
    Util::String dirArg;
    Util::String fileArg;
    Util::String listfileArg;
    bool forceArg;
    bool verboseFlag;
private:
    Util::String GetAdditionalArguments();
    void DeleteDirectoryContent(const IO::URI & path);
    Ptr<SharedDirControl> sharedDirControl;
    Ptr<SharedDirCreator> sharedDirCreator;
    Util::String shareddirArg;
    bool slaveArg;
    bool masterMode;
    bool distributedArg;
    int jobsArg;
    int processorsArg;
};
inline
bool
DistributedToolkitApp::IsDistributed()
{
    return this->distributedArg;
}
inline
bool
DistributedToolkitApp::IsSlave()
{
    return this->slaveArg;
}
inline bool
DistributedToolkitApp::IsMaster()
{
    return this->masterMode;
}
inline
const Ptr<SharedDirControl> &
DistributedToolkitApp::GetSharedDirControl()
{
    return this->sharedDirControl;
}
}