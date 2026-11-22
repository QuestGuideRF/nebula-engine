#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
#include "base/applauncherbase.h"
namespace ToolkitUtil
{
class Win32AppLauncher : public AppLauncherBase
{
public:
    Win32AppLauncher();
    bool LaunchWait();
    bool Launch();
    bool IsRunning();
    void UpdateStdoutStream();
    bool CheckIfExists();
    static bool CheckIfExists(const IO::URI & program);
private:
    bool LaunchWithoutOutputCapturing(DWORD creationFlags, const Util::String& cmdLine) const;
    bool LaunchWithOutputCapturing(DWORD creationFlags, const Util::String& cmdLine) const;
    bool CreateStdoutCapturePipe(PHANDLE stdoutRead, PHANDLE stdoutWrite) const;
    void CaptureOutput(HANDLE stdoutRead, HANDLE stderrRead, HANDLE childProcess) const;
    PROCESS_INFORMATION lauchedProcessInfo;
    HANDLE stdoutAsyncRead;
    HANDLE stdoutAsyncWrite;
    HANDLE stderrAsyncRead;
    HANDLE stderrAsyncWrite;
    DWORD numAsyncBytesRead;
    DWORD numAsyncBytesAvailable;
    static const int asyncBufferSize = 4096;
    char asyncBuffer[asyncBufferSize];
};
}