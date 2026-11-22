#pragma once
#include "game/featureunit.h"
#include "core/cvar.h"
namespace BaseGameFeature
{
class BaseGameFeatureUnit : public Game::FeatureUnit
{
    __DeclareClass(BaseGameFeatureUnit)
    __DeclareSingleton(BaseGameFeatureUnit)
public:
    BaseGameFeatureUnit();
    virtual ~BaseGameFeatureUnit();
    void OnAttach() override;
    void OnActivate() override;
    void OnDeactivate() override;
    void OnEndFrame() override;
    void OnRenderDebug() override;
    void OnFrame() override;
protected:
    Ptr<Game::Manager> blueprintManager;
    Ptr<Game::Manager> timeManager;
    Core::CVar* cl_debug_worlds;
};
}