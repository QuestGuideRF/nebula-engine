#pragma once
#include "toolkitutil/toolkitapp.h"
namespace Toolkit
{
class ArchiverApp : public ToolkitUtil::ToolkitApp
{
public:
    ArchiverApp();
    virtual void Run();
private:
    virtual bool ParseCmdLineArgs();
    virtual bool SetupProjectInfo();
    virtual void ShowHelp();
    void PackWebDeploy(const Util::String& dir, const Util::String& webDeployDir);
    void PackDirectoryWin360(const Util::String& dir);
    void RecursePackWebDeployDirectory(const Util::String& srcDir, const Util::String& dstDir);
    void CompressCopyFile(const Util::String& srcPath, const Util::String& dstPath);
    Util::String toolPath;
    Util::String wiiDvdRoot;
    Util::Array<Util::String> excludePatterns;
    bool webDeployFlag;
};
}