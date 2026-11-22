#pragma once
#include "GLFW/glfw3.h"
#include "util/fixedarray.h"
#include "coregraphics/window.h"
#include "coregraphics/displaymode.h"
#include "coregraphics/swapchain.h"
#include "ids/idallocator.h"
namespace CoreGraphics
{
struct ResizeInfo
{
    uint newWidth : 16, newHeight : 16;
    bool done : 1;
    bool vsync : 1;
};
enum
{
    GLFW_Window,
    GLFW_DisplayMode,
    GLFW_SwapFrame,
    GLFW_SetupInfo,
    GLFW_ResizeInfo,
    GLFW_Swapchain
};
typedef Ids::IdAllocator<
      GLFWwindow*
    , CoreGraphics::DisplayMode
    , IndexT
    , WindowCreateInfo
    , ResizeInfo
    , SwapchainId
> GLFWWindowAllocatorType;
extern GLFWWindowAllocatorType glfwWindowAllocator;
}
namespace GLFW
{
void KeyFunc(const CoreGraphics::WindowId& id, int key, int scancode, int action, int mods);
void CharFunc(const CoreGraphics::WindowId& id, unsigned int key);
void MouseButtonFunc(const CoreGraphics::WindowId& id, int button, int action, int mods);
void MouseFunc(const CoreGraphics::WindowId& id, double xpos, double ypos);
void ScrollFunc(const CoreGraphics::WindowId& id, double xs, double ys);
void CloseFunc(const CoreGraphics::WindowId& id);
void FocusFunc(const CoreGraphics::WindowId& id, int focus);
void ResizeFunc(const CoreGraphics::WindowId& id, int width, int height);
void staticKeyFunc(GLFWwindow *window, int key, int scancode, int action, int mods);
void staticMouseButtonFunc(GLFWwindow *window, int button, int action, int mods);
void staticMouseFunc(GLFWwindow *window, double xpos, double ypos);
void staticScrollFunc(GLFWwindow *window, double xs, double ys);
void staticCloseFunc(GLFWwindow * window);
void staticFocusFunc(GLFWwindow * window, int focus);
void staticSizeFunc(GLFWwindow* window, int width, int height);
void staticCharFunc(GLFWwindow* window, unsigned int key);
void EnableCallbacks(const CoreGraphics::WindowId& id);
void DisableCallbacks(const CoreGraphics::WindowId& id);
const CoreGraphics::WindowId InternalSetupFunction(const CoreGraphics::WindowCreateInfo& info, const Util::Blob& windowData, bool embed);
}