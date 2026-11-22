#pragma once
#include "game/featureunit.h"
#include "graphics/graphicsserver.h"
#include "graphics/view.h"
#include "graphics/stage.h"
#include "input/inputserver.h"
#include "managers/cameramanager.h"
#include "core/cvar.h"
#include "terrain/terraincontext.h"
namespace GraphicsFeature
{
class GraphicsFeatureUnit : public Game::FeatureUnit
{
    __DeclareClass(GraphicsFeatureUnit)
    __DeclareSingleton(GraphicsFeatureUnit)
public:
    GraphicsFeatureUnit();
    ~GraphicsFeatureUnit();
    void OnAttach() override;
    void OnActivate() override;
    void OnDeactivate() override;
    void OnBeginFrame() override;
    void OnBeforeViews() override;
    void OnFrame() override;
    void OnEndFrame() override;
    void OnRenderDebug() override;
    void SetWindowTitle(const Util::StringAtom& title);
    void SetGraphicsDebugging(bool value);
    Ptr<Graphics::View> GetDefaultView() const;
    Ptr<Graphics::Stage> GetDefaultStage() const;
    ViewHandle GetDefaultViewHandle() const;
    void SetFrameScript(IO::URI const& uri);
    void SetupTerrainBiome(const Terrain::BiomeSettings& biomeParameters);
    Graphics::GraphicsEntityId globalLight;
private:
    IO::URI defaultFrameScript;
    Util::StringAtom title;
    Ptr<Graphics::View> defaultView;
    Ptr<Graphics::Stage> defaultStage;
    Ptr<Graphics::GraphicsServer> gfxServer;
    Ptr<Input::InputServer> inputServer;
    CoreGraphics::WindowId wnd;
    Ptr<Game::Manager> graphicsManager;
    Ptr<Game::Manager> cameraManager;
    struct TerrainInstance
    {
        Graphics::GraphicsEntityId entity;
        Util::Array<Terrain::TerrainBiomeId> biomes;
    } terrain;
    ViewHandle defaultViewHandle;
    Core::CVar* r_debug;
    Core::CVar* r_show_frame_inspector;
};
inline void
GraphicsFeature::GraphicsFeatureUnit::SetGraphicsDebugging(bool value)
{
    Core::CVarWriteInt(this->r_debug, 2);
}
inline void
GraphicsFeatureUnit::SetWindowTitle(const Util::StringAtom& title)
{
    this->title = title;
}
inline Ptr<Graphics::View>
GraphicsFeatureUnit::GetDefaultView() const
{
    return this->defaultView;
}
inline Ptr<Graphics::Stage>
GraphicsFeatureUnit::GetDefaultStage() const
{
    return this->defaultStage;
}
inline ViewHandle
GraphicsFeatureUnit::GetDefaultViewHandle() const
{
    return this->defaultViewHandle;
}
inline void
GraphicsFeatureUnit::SetFrameScript(IO::URI const& uri)
{
    this->defaultFrameScript = uri;
}
}