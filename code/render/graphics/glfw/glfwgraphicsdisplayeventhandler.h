#pragma once
#include "coregraphics/displayeventhandler.h"
namespace GLFW
{
class GLFWGraphicsDisplayEventHandler : public CoreGraphics::DisplayEventHandler
{
    __DeclareClass(GLFWGraphicsDisplayEventHandler);
public:
    virtual bool HandleEvent(const CoreGraphics::DisplayEvent& event);
};
}