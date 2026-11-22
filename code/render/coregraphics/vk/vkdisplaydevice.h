#pragma once
#include "core/refcounted.h"
#include "coregraphics/glfw/glfwdisplaydevice.h"
namespace Vulkan
{
class VkDisplayDevice : public GLFW::GLFWDisplayDevice
{
    __DeclareClass(VkDisplayDevice);
    __DeclareSingleton(VkDisplayDevice);
public:
    VkDisplayDevice();
    virtual ~VkDisplayDevice();
private:
    friend class VkRenderDevice;
    bool WindowOpen();
    bool EmbedWindow();
    void SetupSwapchain();
    VkSurfaceKHR surface;
};
}