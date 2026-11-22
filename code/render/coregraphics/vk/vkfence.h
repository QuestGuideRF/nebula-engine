#pragma once
#include "ids/idallocator.h"
#include "coregraphics/fence.h"
namespace Vulkan
{
struct VkFenceInfo
{
    VkFence fence;
};
typedef Ids::IdAllocator<VkDevice, VkFenceInfo> VkFenceAllocator;
extern VkFenceAllocator fenceAllocator;
VkFence FenceGetVk(const CoreGraphics::FenceId id);
}