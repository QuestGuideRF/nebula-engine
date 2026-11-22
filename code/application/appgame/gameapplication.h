#pragma once
#include "app/application.h"
#include "game/gameserver.h"
#include "core/singleton.h"
#include "core/coreserver.h"
#include "debug/debuginterface.h"
#include "io/ioserver.h"
#include "io/iointerface.h"
#include "io/gamecontentserver.h"
#include "resources/resourceserver.h"
#include "http/httpinterface.h"
#include "http/httpserverproxy.h"
#include "basegamefeature/basegamefeatureunit.h"
namespace App
{
class GameApplication : public Application
{
    __DeclareSingleton(GameApplication)
public:
    GameApplication();
    virtual ~GameApplication();
    virtual bool Open();
    virtual void Close();
    virtual void Run();
    virtual void StepFrame();
    static IndexT FrameIndex;
    static bool IsEditorEnabled();
protected:
    virtual void SetupGameFeatures();
    virtual void CleanupGameFeatures();
    virtual void SetupAppFromCmdLineArgs();
    Ptr<Core::CoreServer> coreServer;
    Ptr<IO::GameContentServer> gameContentServer;
    Ptr<Resources::ResourceServer> resourceServer;
    Ptr<IO::IoServer> ioServer;
    Ptr<IO::IoInterface> ioInterface;
    Ptr<BaseGameFeature::BaseGameFeatureUnit> baseGameFeature;
    static bool editorEnabled;
#if __NEBULA_HTTP__
    Ptr<Debug::DebugInterface> debugInterface;
    Ptr<Http::HttpInterface> httpInterface;
    Ptr<Http::HttpServerProxy> httpServerProxy;
    ushort defaultTcpPort;
#endif
    Ptr<Game::GameServer> gameServer;
    _declare_timer(GameApplicationFrameTimeAll);
    class GameAppExitHandler : public Core::ExitHandler
    {
    public:
        GameAppExitHandler(GameApplication* app) : app(app), Core::ExitHandler()
        {
        }
    private:
        virtual void OnExit() const
        {
            if (app->isOpen)
                app->Close();
        }
        GameApplication* app;
    };
    GameAppExitHandler exitHandler;
};
inline
bool GameApplication::IsEditorEnabled()
{
    return editorEnabled;
}
}