#pragma once
#include "core/refcounted.h"
#include "editor/tools/camera.h"
namespace Graphics{ class Stage; class View; }
namespace Presentation
{
namespace Modules
{
class Viewport
{
public:
    enum RenderMode
    {
        TexturedLit,
        TexturedUnlit,
        SolidLit,
        SolidUnlit,
        Wireframe,
    };
    Viewport();
    ~Viewport();
    void Init(Util::String const& viewName);
    void Init(Ptr<Graphics::View> const& view);
    void Render();
    void SetStage(Ptr<Graphics::Stage> const& stage);
    void SetFrameBuffer(Util::String const& name);
    void Update();
    bool IsFocused() const
    {
        return this->focused;
    }
    const Ptr<Graphics::View> GetView() const;
    Editor::Camera camera;
    Math::vec2 lastViewportImagePosition;
    Math::vec2 lastViewportImageSize;
private:
    RenderMode renderMode = TexturedLit;
    Ptr<Graphics::Stage> stage;
    Ptr<Graphics::View> view;
    Resources::ResourceId resourceId;
    bool focused = false;
    Util::String frameBuffer;
};
}
}