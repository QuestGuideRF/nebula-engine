#pragma once
#include "app/application.h"
#include "core/coreserver.h"
#include "jobs/jobsystem.h"
#include "debug/debuginterface.h"
#include "io/ioserver.h"
#include "io/iointerface.h"
#include "graphics/graphicsinterface.h"
#include "graphics/display.h"
#include "timing/timer.h"
#include "debug/debuginterface.h"
#include "debug/debugtimer.h"
#include "input/inputserver.h"
#include "io/gamecontentserver.h"
#include "util/delegate.h"
#include "messaging/message.h"
#if __NEBULA_HTTP__
#include "http/httpinterface.h"
#include "http/httpserverproxy.h"
#endif
#if __NEBULA_HTTP_FILESYSTEM__
#include "http/httpclientregistry.h"
#endif
namespace App
{
class RenderApplication : public Application
{
public:
    RenderApplication();
    virtual ~RenderApplication();
    void SetLogFileEnabled(bool b);
    void SetOverrideRootDirectory(const Util::String& rootDir);
    void SetMountStandardArchivesEnabled(bool b);
    void SetMultithreadedRendering(bool b);
    void SetWindowData(const Util::Blob& data);
    virtual bool Open();
    virtual void Close();
    virtual void Run();
private:
    void UpdateTime();
protected:
    Util::String LookupProjectDirectory();
    void SetQuitRequested(bool b);
    bool IsQuitRequested() const;
    virtual void OnConfigureDisplay();
    virtual void OnSetupResourceMappers();
    virtual void OnProcessInput();
    virtual void OnUpdateFrame();
    Timing::Time GetTime() const;
    Timing::Time GetFrameTime() const;
    Ptr<Core::CoreServer> coreServer;
    Ptr<Jobs::JobSystem> jobSystem;
    Ptr<IO::GameContentServer> gameContentServer;
    Ptr<IO::IoServer> ioServer;
    Ptr<IO::IoInterface> ioInterface;
    Ptr<Graphics::GraphicsInterface> graphicsInterface;
    Ptr<Graphics::Display> display;
    Ptr<Input::InputServer> inputServer;
    Ptr<FrameSync::FrameSyncTimer> frameSyncTimer;
    Ptr<Resources::ResourceManager> resManager;
    Util::Blob windowData;
#if __NEBULA_HTTP__
    Ptr<Debug::DebugInterface> debugInterface;
    Ptr<Http::HttpInterface> httpInterface;
    Ptr<Http::HttpServerProxy> httpServerProxy;
#endif
#if __NEBULA_HTTP_FILESYSTEM__
    Ptr<Http::HttpClientRegistry> httpClientRegistry;
#endif
    Timing::Time time;
    Timing::Time frameTime;
    bool useMultithreadedRendering;
    bool quitRequested;
    bool logFileEnabled;
    bool mountStandardArchivesEnabled;
    Util::String overrideRootDirectory;
    _declare_timer(MainThreadFrameTimeAll);
    _declare_timer(MainThreadWaitForGraphicsFrame);
};
inline void
RenderApplication::SetLogFileEnabled(bool b)
{
    this->logFileEnabled = b;
}
inline void
RenderApplication::SetMountStandardArchivesEnabled(bool b)
{
    this->mountStandardArchivesEnabled = b;
}
inline void
RenderApplication::SetMultithreadedRendering( bool b )
{
    this->useMultithreadedRendering = b;
}
inline void
RenderApplication::SetWindowData(const Util::Blob& data)
{
    this->windowData = data;
}
inline void
RenderApplication::SetQuitRequested(bool b)
{
    this->quitRequested = b;
}
inline bool
RenderApplication::IsQuitRequested() const
{
    return this->quitRequested;
}
inline Timing::Time
RenderApplication::GetTime() const
{
    return this->time;
}
inline Timing::Time
RenderApplication::GetFrameTime() const
{
    return this->frameTime;
}
inline void
RenderApplication::SetOverrideRootDirectory(const Util::String& d)
{
    this->overrideRootDirectory = d;
}
}