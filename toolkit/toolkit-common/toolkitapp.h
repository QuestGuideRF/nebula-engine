#pragma once
#include "app/consoleapplication.h"
#include "projectinfo.h"
#include "platform.h"
#include "logger.h"
#include "toolkitversion.h"
#include "toolkitconsolehandler.h"
namespace ToolkitUtil
{
class ToolkitApp : public App::ConsoleApplication
{
public:
    ToolkitApp();
    virtual bool Open();
    virtual void Close();
    const Util::String& GetToolkitVersion() const;
protected:
    virtual bool ParseCmdLineArgs();
    virtual bool SetupProjectInfo();
    virtual void ShowHelp();
    Logger logger;
    ProjectInfo projectInfo;
    Util::String toolkitVersion;
    bool waitForKey;
    Platform::Code platform;
    Ptr<ToolkitUtil::ToolkitConsoleHandler> handler;
};
inline const Util::String &
ToolkitApp::GetToolkitVersion() const
{
    return this->toolkitVersion;
}
}