#pragma once
#include "toolkit-common/distributedtools/distributedtoolkitapp.h"
#include "toolkit-common/toolkitconsolehandler.h"
namespace ToolkitUtil
{
class ModelDatabase;
}
namespace Toolkit
{
class AssetBatcherApp : public DistributedTools::DistributedToolkitApp
{
public:
    AssetBatcherApp();
    virtual ~AssetBatcherApp();
    bool Open();
    void Close();
    void Run() override;
    void DoWork();
protected:
    virtual Util::Array<Util::String> CreateFileList();
    virtual void OnBeforeRunLocal();
private:
    bool ParseCmdLineArgs();
    bool SetupProjectInfo();
    void ShowHelp();
    Ptr<ToolkitUtil::ModelDatabase> modelDatabase;
    Ptr<ToolkitUtil::ToolkitConsoleHandler> handler;
};
}