#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
#include "system/base/processbase.h"
namespace Win32
{
class Win32Process : public Base::Process
{
public:
    Win32Process();
    bool LaunchWait() const;
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