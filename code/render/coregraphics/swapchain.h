#pragma once
#include "coregraphics/config.h"
#include "coregraphics/displaydevice.h"
#include "ids/idallocator.h"
namespace CoreGraphics
{
ID_24_8_TYPE(SwapchainId);
struct SwapchainCreateInfo
{
    CoreGraphics::DisplayMode displayMode;
    bool vsync;
    GLFWwindow* window;
    CoreGraphics::QueueType preferredQueue = CoreGraphics::ComputeQueueType;
};
SwapchainId CreateSwapchain(const SwapchainCreateInfo& info);
void DestroySwapchain(const SwapchainId id);
void SwapchainSwap(const SwapchainId id);
CoreGraphics::QueueType SwapchainGetQueueType(const SwapchainId id);
CoreGraphics::CmdBufferId SwapchainAllocateCmds(const SwapchainId id);
void SwapchainCopy(const SwapchainId id, const CoreGraphics::CmdBufferId cmdBuf, const CoreGraphics::TextureId source);
void SwapchainPresent(const SwapchainId id);
}