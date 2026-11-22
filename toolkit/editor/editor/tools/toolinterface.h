#pragma once
#include "editor/editor.h"
namespace Editor
{
class Camera;
}
namespace Tools
{
class ToolInterface
{
public:
    virtual void Render(Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera) = 0;
    virtual void Update(Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera) = 0;
    virtual bool IsModifying() const = 0;
    virtual void Abort() {}
};
}