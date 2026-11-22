#pragma once
#include "distributedtools/distributedtoolkitapp.h"
#include "toolkitutil/modelutil/modelbuilder.h"
#include "modelutil/modeldatabase.h"
namespace Toolkit
{
class ModelBuilder3App : public DistributedTools::DistributedToolkitApp
{
public:
    ModelBuilder3App();
    virtual ~ModelBuilder3App();
    void DoWork();
    bool Open();
protected:
    virtual Util::Array<Util::String> CreateFileList();
private:
    void Convert();
    virtual bool ParseCmdLineArgs();
    virtual bool SetupProjectInfo();
    virtual void ShowHelp();
    Ptr<ToolkitUtil::ModelDatabase> database;
    Ptr<ToolkitUtil::ModelBuilder> builder;
};
}