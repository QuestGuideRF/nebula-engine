#pragma once
#include "toolinterface.h"
#include "math/plane.h"
namespace Editor
{
class Camera;
}
namespace Tools
{
class RotateTool : public ToolInterface
{
public:
    void Render(Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera) override;
    void Update(Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera) override;
    bool IsModifying() const override;
    void Abort() override;
private:
    struct
    {
        bool isDirty = false;
        bool gizmoTransforming = false;
    } translation;
};
}