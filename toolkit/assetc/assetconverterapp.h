#pragma once
#include "toolkit-common/toolkitapp.h"
namespace ToolkitUtil
{
class ModelDatabase;
}
namespace Toolkit
{
class AssetConverterApp : public ToolkitUtil::ToolkitApp
{
public:
    AssetConverterApp();
    virtual ~AssetConverterApp();
    virtual void Run();
private:
    void ShowHelp();
    Ptr<ToolkitUtil::ModelDatabase> modelDatabase;
};
}