#pragma once
#include "core/singleton.h"
#include "editor/editor.h"
namespace Editor
{
class Camera;
}
namespace Edit
{
struct CMDSetSelection;
}
namespace Tools
{
class SelectionContext
{
    __DeclareSingleton(SelectionContext)
public:
    static void Create();
    static void Destroy();
    static Util::Array<Editor::Entity> const& Selection();
    static void PerformPicking(Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera);
    static void ValidateSelection();
    static void Pause();
    static void Unpause();
    static bool IsPaused();
    static Editor::Entity GetSelectedEntityUnderMouse(
        Math::vec2 const& viewPortPosition, Math::vec2 const& viewPortSize, Editor::Camera const* camera
    );
private:
    SelectionContext();
    ~SelectionContext();
    friend Edit::CMDSetSelection;
    Util::Array<Editor::Entity> selection;
    int pauseCounter = 0;
};
}