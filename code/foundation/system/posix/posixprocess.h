#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
#include "system/base/processbase.h"
namespace Posix
{
class PosixProcess : public Base::Process
{
public:
    PosixProcess();
    bool LaunchWait() const;
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