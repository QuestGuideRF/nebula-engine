#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "game/manager.h"
#include "game/category.h"
#include "graphics/graphicsentity.h"
#include "game/componentinspection.h"
#include "components/camera.h"
#include "components/decal.h"
#include "components/lighting.h"
#include "components/model.h"
#include "components/gi.h"
namespace GraphicsFeature
{
class GraphicsManager : public Game::Manager
{
    __DeclareClass(GraphicsManager)
public:
    GraphicsManager();
    virtual ~GraphicsManager();
    void OnActivate() override;
    void OnDeactivate() override;
    void OnDecay() override;
    void OnCleanup(Game::World* world) override;
    static void InitModel(Game::World*, Game::Entity, Model*);
    static void InitPointLight(Game::World* world, Game::Entity entity, PointLight* light);
    static void InitSpotLight(Game::World* world, Game::Entity entity, SpotLight* light);
    static void InitAreaLight(Game::World* world, Game::Entity entity, AreaLight* light);
    static void InitDecal(Game::World* world, Game::Entity entity, Decal* decal);
    static void InitDDGIVolume(Game::World* world, Game::Entity entity, DDGIVolume* volume);
private:
    void InitUpdateModelTransformProcessor();
    void InitUpdateLightTransformProcessor();
    void InitUpdateDecalTransformProcessor();
    void InitUpdateDDGIVolumeTransformProcessor();
};
}
namespace Game
{
template <>
void ComponentDrawFuncT<GraphicsFeature::AreaLightShape>(Game::Entity, ComponentId, void*, bool*);
}