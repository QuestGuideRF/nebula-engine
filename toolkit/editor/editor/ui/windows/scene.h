#pragma once
#include "core/refcounted.h"
#include "editor/ui/window.h"
#include "editor/ui/modules/viewport.h"
namespace Tools
{
class ToolInterface;
}
namespace Presentation
{
class Scene : public BaseWindow
{
    __DeclareClass(Scene)
public:
    Scene();
    ~Scene();
    void Update();
    void Run(SaveMode save) override;
    void FocusCamera();
    void DrawOutlines();
    Modules::Viewport viewPort;
    enum ToolType
    {
        TOOL_SELECTION,
        TOOL_TRANSLATE,
        TOOL_ROTATE,
        TOOL_SCALE
    };
    void SetTool(ToolType type);
private:
    Tools::ToolInterface* currentTool;
    Tools::ToolInterface* allTools[4];
};
__RegisterClass(Scene)
}