#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
namespace Base
{
class Process
{
public:
    Process();
    void SetExecutable(const IO::URI& uri);
    void SetWorkingDirectory(const IO::URI& uri);
    void SetArguments(const Util::String& args);
    void SetNoConsoleWindow(bool b);
    void SetStdoutCaptureStream(const Ptr<IO::Stream>& stream);
    const Ptr<IO::Stream>& GetStdoutCaptureStream() const;
    void SetStderrCaptureStream(const Ptr<IO::Stream>& stream);
    const Ptr<IO::Stream>& GetStderrCaptureStream() const;
    virtual bool LaunchWait() const = 0;
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
Process::Process() :
    noConsoleWindow(false),
    isRunning(false),
    stdoutCaptureStream(nullptr)
{
}
inline void
Process::SetExecutable(const IO::URI& uri)
{
    this->exePath = uri;
}
inline void
Process::SetWorkingDirectory(const IO::URI& uri)
{
    this->workingDir = uri;
}
inline void
Process::SetArguments(const Util::String& a)
{
    this->args = a;
}
inline void
Process::SetNoConsoleWindow(bool b)
{
    this->noConsoleWindow = b;
}
inline void
Process::SetStdoutCaptureStream(const Ptr<IO::Stream>& stream)
{
    this->stdoutCaptureStream = stream;
}
inline const Ptr<IO::Stream>&
Process::GetStdoutCaptureStream() const
{
    return this->stdoutCaptureStream;
}
inline void
Process::SetStderrCaptureStream(const Ptr<IO::Stream>& stream)
{
    this->stderrCaptureStream = stream;
}
inline const Ptr<IO::Stream>&
Process::GetStderrCaptureStream() const
{
    return this->stderrCaptureStream;
}
}