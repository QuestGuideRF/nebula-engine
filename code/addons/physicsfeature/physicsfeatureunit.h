#pragma once
#include "game/featureunit.h"
#include "core/cvar.h"
#define TIMESOURCE_PHYSICS 'PHTS'
namespace PhysicsFeature
{
class PhysicsFeatureUnit : public Game::FeatureUnit
{
    __DeclareClass(PhysicsFeatureUnit)
    __DeclareSingleton(PhysicsFeatureUnit)
public:
    PhysicsFeatureUnit();
    ~PhysicsFeatureUnit();
    void OnAttach() override;
    void OnActivate();
    void OnDeactivate();
    void OnBeforeCleanup(Game::World* world);
    virtual void OnBeginFrame();
    virtual void OnDecay();
    virtual void OnRenderDebug();
private:
    Util::Dictionary<Game::World*, IndexT> physicsWorlds;
    bool simulating;
    int debugDrawPhysicsLastValue = 0;
    Core::CVar* cl_debug_draw_physics;
};
}