#pragma once
#include "core/singleton.h"
#include "core/refcounted.h"
#include "toolkit-common/distributedtools/shareddircontrol.h"
#include "toolkit-common/projectinfo.h"
#include "util/guid.h"
namespace DistributedTools
{
class SharedDirCreator : public Core::RefCounted
{
    __DeclareClass(SharedDirCreator);
    __DeclareSingleton(SharedDirCreator);
public:
    SharedDirCreator();
    virtual ~SharedDirCreator();
    Ptr<SharedDirControl> CreateSharedControlObject(const Util::String & path);
    Ptr<SharedDirControl> CreateSharedControlObject(const Util::String & path, const Util::Guid & guid);
    void SetProjectInfo(const ToolkitUtil::ProjectInfo & info);
private:
    ToolkitUtil::ProjectInfo projectInfo;
};
inline
void
SharedDirCreator::SetProjectInfo(const ToolkitUtil::ProjectInfo & info)
{
    this->projectInfo = info;
}
}