#pragma once
#include "core/singleton.h"
#include "util/commandlineargs.h"
namespace App
{
class Application
{
    __DeclareSingleton(Application);
public:
    Application();
    virtual ~Application();
    void SetCompanyName(const Util::String& n);
    const Util::String& GetCompanyName() const;
    void SetAppTitle(const Util::String& n);
    const Util::String& GetAppTitle() const;
    void SetAppID(const Util::String& n);
    const Util::String& GetAppID() const;
    void SetAppVersion(const Util::String& n);
    const Util::String& GetAppVersion() const;
    void SetCmdLineArgs(const Util::CommandLineArgs& a);
    const Util::CommandLineArgs& GetCmdLineArgs() const;
    virtual bool Open();
    virtual void Close();
    virtual void Exit();
    virtual void Run();
    bool IsOpen() const;
    int GetReturnCode() const;
protected:
    void SetReturnCode(int c);
    Util::String companyName;
    Util::String appName;
    Util::String appID;
    Util::String appVersion;
    Util::CommandLineArgs args;
    bool isOpen;
    int returnCode;
};
inline
bool
Application::IsOpen() const
{
    return this->isOpen;
}
inline
void
Application::SetCompanyName(const Util::String& n)
{
    this->companyName = n;
}
inline
const Util::String&
Application::GetCompanyName() const
{
    return this->companyName;
}
inline
void
Application::SetAppTitle(const Util::String& n)
{
    this->appName = n;
}
inline
const Util::String&
Application::GetAppTitle() const
{
    return this->appName;
}
inline
void
Application::SetAppID(const Util::String& n)
{
    this->appID = n;
}
inline
const Util::String&
Application::GetAppID() const
{
    return this->appID;
}
inline
void
Application::SetAppVersion(const Util::String& n)
{
    this->appVersion = n;
}
inline
const Util::String&
Application::GetAppVersion() const
{
    return this->appVersion;
}
inline
void
Application::SetCmdLineArgs(const Util::CommandLineArgs& a)
{
    this->args = a;
}
inline
const Util::CommandLineArgs&
Application::GetCmdLineArgs() const
{
    return this->args;
}
inline void
Application::SetReturnCode(int c)
{
    this->returnCode = c;
}
inline int
Application::GetReturnCode() const
{
    return this->returnCode;
}
}