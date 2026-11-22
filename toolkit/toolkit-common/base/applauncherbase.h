#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
namespace ToolkitUtil
{
class AppLauncherBase
{
public:
    AppLauncherBase();
    void SetExecutable(const IO::URI& uri);
    void SetWorkingDirectory(const IO::URI& uri);
    void SetArguments(const Util::String& args);
    void SetNoConsoleWindow(bool b);
    void SetStdoutCaptureStream(const Ptr<IO::Stream>& stream);
    const Ptr<IO::Stream>& GetStdoutCaptureStream() const;
    void SetStderrCaptureStream(const Ptr<IO::Stream>& stream);
    const Ptr<IO::Stream>& GetStderrCaptureStream() const;
    virtual bool LaunchWait() = 0;
    virtual bool Launch() = 0;
    virtual bool IsRunning() = 0;
    virtual void UpdateStdoutStream() = 0;
    virtual bool CheckIfExists() = 0;
protected:
    bool noConsoleWindow;
    bool isRunning;
    IO::URI exePath;
    IO::URI workingDir;
    Util::String args;
    Ptr<IO::Stream> stdoutCaptureStream;
    Ptr<IO::Stream> stderrCaptureStream;
};
inline
AppLauncherBase::AppLauncherBase() :
    noConsoleWindow(false),
    isRunning(false),
    stdoutCaptureStream(nullptr)
{
}
inline void
AppLauncherBase::SetExecutable(const IO::URI& uri)
{
    this->exePath = uri;
}
inline void
AppLauncherBase::SetWorkingDirectory(const IO::URI& uri)
{
    this->workingDir = uri;
}
inline void
AppLauncherBase::SetArguments(const Util::String& a)
{
    this->args = a;
}
inline void
AppLauncherBase::SetNoConsoleWindow(bool b)
{
    this->noConsoleWindow = b;
}
inline void
AppLauncherBase::SetStdoutCaptureStream(const Ptr<IO::Stream>& stream)
{
    this->stdoutCaptureStream = stream;
}
inline const Ptr<IO::Stream>&
AppLauncherBase::GetStdoutCaptureStream() const
{
    return this->stdoutCaptureStream;
}
inline void
AppLauncherBase::SetStderrCaptureStream(const Ptr<IO::Stream>& stream)
{
    this->stderrCaptureStream = stream;
}
inline const Ptr<IO::Stream>&
AppLauncherBase::GetStderrCaptureStream() const
{
    return this->stderrCaptureStream;
}
}