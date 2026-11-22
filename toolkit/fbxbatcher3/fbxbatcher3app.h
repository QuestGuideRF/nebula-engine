#pragma once
#include "toolkit-common/distributedtools/distributedtoolkitapp.h"
#include "modelutil/modeldatabase.h"
namespace Toolkit
{
class FBXBatcher3App : public DistributedTools::DistributedToolkitApp
{
public:
    bool Open();
    void DoWork();
    FBXBatcher3App();
    virtual ~FBXBatcher3App();
protected:
    virtual Util::Array<Util::String> CreateFileList();
private:
    bool ParseCmdLineArgs();
    bool SetupProjectInfo();
    void ShowHelp();
    Ptr<ToolkitUtil::ModelDatabase> modelDatabase;
};
}