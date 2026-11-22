#pragma once
#include "core/refcounted.h"
#include "frame/framescript.h"
#include "timing/time.h"
#include "graphicsentity.h"
#include "stage.h"
namespace Graphics
{
class Stage;
class Camera;
class View : public Core::RefCounted
{
    __DeclareClass(View);
public:
    View();
    virtual ~View();
    void UpdateConstants();
    void Render(const IndexT frameIndex, const Timing::Time time, const IndexT bufferIndex);
    void SetFrameScript(void(*func)(const Math::rectangle<int>& viewport, IndexT frameIndex, IndexT bufferIndex));
    void SetViewport(const Math::rectangle<int>& rect);
    const Math::rectangle<int>& GetViewport();
    void SetCamera(const GraphicsEntityId& camera);
    const GraphicsEntityId& GetCamera();
    void SetStage(const Ptr<Stage>& stage);
    const Ptr<Stage>& GetStage() const;
    bool IsEnabled() const;
    void Enable();
    void Disable();
private:
    friend class GraphicsServer;
    Math::rectangle<int> viewport;
    void (*func)(const Math::rectangle<int>& viewport, IndexT frameIndex, IndexT bufferIndex);
    GraphicsEntityId camera;
    Ptr<Stage> stage;
    bool enabled;
};
inline void
View::SetCamera(const GraphicsEntityId& camera)
{
    this->camera = camera;
}
inline const GraphicsEntityId&
Graphics::View::GetCamera()
{
    return this->camera;
}
inline void
View::SetStage(const Ptr<Stage>& stage)
{
    this->stage = stage;
}
inline const Ptr<Stage>&
View::GetStage() const
{
    return this->stage;
}
inline bool
View::IsEnabled() const
{
    return this->enabled;
}
inline void
View::Enable()
{
    this->enabled = true;
}
inline void
View::Disable()
{
    this->enabled = false;
}
inline void
View::SetFrameScript(void(*func)(const Math::rectangle<int>& viewport, IndexT frameIndex, IndexT bufferIndex))
{
    this->func = func;
}
inline void
View::SetViewport(const Math::rectangle<int>& rect)
{
    this->viewport = rect;
}
inline const
Math::rectangle<int>& View::GetViewport()
{
    return this->viewport;
}
}