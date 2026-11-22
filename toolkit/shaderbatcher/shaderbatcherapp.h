#pragma once
#include "toolkit-common/toolkitapp.h"
#include "shadercompiler.h"
namespace Toolkit
{
class ShaderBatcherApp : public ToolkitUtil::ToolkitApp
{
public:
    virtual void Run();
private:
    virtual bool ParseCmdLineArgs();
    virtual bool SetupProjectInfo();
    virtual void ShowHelp();
    ToolkitUtil::ShaderCompiler shaderCompiler;
};
}