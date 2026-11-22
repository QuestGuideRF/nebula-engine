#pragma once
#include "distributedtools/distributedtoolkitapp.h"
namespace Tools
{
class SetupApplication : public ToolkitUtil::ToolkitApp
{
public:
    SetupApplication();
    virtual ~SetupApplication();
    bool Open();
    void DoWork();
private:
    bool ParseCmdLineArgs();
    bool SetupProjectInfo();
    void ShowHelp();
};
}