#pragma once
#if __VULKAN__
#include "coregraphics/glfw/glfwdisplaydevice.h"
namespace CoreGraphics
{
class DisplayDevice : public GLFW::GLFWDisplayDevice
{
    __DeclareClass(DisplayDevice);
    __DeclareSingleton(DisplayDevice);
public:
    DisplayDevice();
    virtual ~DisplayDevice();
};
}
#else
#error "CoreGraphics::DisplayDevice не реализован на этой платформе!"
#endif