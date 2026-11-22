#pragma once
#include "toolkit-common/toolkitapp.h"
#include "singleshadercompiler.h"
namespace Toolkit
{
class ShaderCompilerApp : public ToolkitUtil::ToolkitApp
{
public:
    virtual void Run();
private:
    virtual bool ParseCmdLineArgs();
    ToolkitUtil::SingleShaderCompiler shaderCompiler;
    Util::String src;
    Util::String type;
};
}