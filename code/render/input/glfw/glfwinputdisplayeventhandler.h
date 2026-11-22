#pragma once
#include "coregraphics/displayeventhandler.h"
namespace GLFW
{
class GLFWInputDisplayEventHandler : public CoreGraphics::DisplayEventHandler
{
    __DeclareClass(GLFWInputDisplayEventHandler);
public:
    virtual bool HandleEvent(const CoreGraphics::DisplayEvent& event);
};
}