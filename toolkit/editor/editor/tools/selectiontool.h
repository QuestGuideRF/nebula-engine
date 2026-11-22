#pragma once
#include "toolinterface.h"
#include "math/plane.h"
namespace Editor
{
class Camera;
}
namespace Tools
{
class SelectionTool : public ToolInterface
{
public:
    void Render(Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera) override;
    void Update(Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera) override;
    bool IsModifying() const override;
    void SnapToGridIncrements(bool value);
    bool SnapToGridIncrements();
private:
    struct
    {
        float size = 1.0f;
    } grid;
    struct
    {
        bool useGridIncrements = false;
        bool isDirty = false;
        Math::vec3 delta = Math::vec3(0);
        Math::vec3 startPos;
        Math::plane plane;
        Math::vec2 mousePosOnStart;
        Timing::Timer dragTimer;
        Editor::Entity originEntity = Editor::Entity::Invalid();
    } translation;
};
}