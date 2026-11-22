#pragma once
#include "toolkitutil/toolkitapp.h"
#include "toolkitutil/audioexporter.h"
namespace Toolkit
{
class AudioBatcherApp : public ToolkitUtil::ToolkitApp
{
public:
    virtual void Run();
private:
    virtual bool ParseCmdLineArgs();
    virtual bool SetupProjectInfo();
    virtual void ShowHelp();
    ToolkitUtil::AudioExporter audioExporter;
};
}