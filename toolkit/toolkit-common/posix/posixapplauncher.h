#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
#include "toolkit-common/base/applauncherbase.h"
namespace ToolkitUtil
{
class PosixAppLauncher : public AppLauncherBase
{
public:
    PosixAppLauncher();
    bool LaunchWait();
    bool Launch();
    bool IsRunning();
    void UpdateStdoutStream();
    bool CheckIfExists();
    static bool CheckIfExists(const IO::URI & program);
private:
    void CleanUp() const;
    int inPipe;
    int outPipe;
    int errPipe;
    pid_t pid;
};
}