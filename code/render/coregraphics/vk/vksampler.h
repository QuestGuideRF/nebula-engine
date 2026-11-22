#pragma once
#include "coregraphics/sampler.h"
#include "ids/idallocator.h"
namespace Vulkan
{
typedef Ids::IdAllocator<
    VkDevice,
    VkSampler,
    uint32_t
> VkSamplerAllocator;
extern VkSamplerAllocator samplerAllocator;
const VkSampler& SamplerGetVk(const CoreGraphics::SamplerId& id);
}
namespace CoreGraphics
{
SamplerCreateInfo ToNebulaSamplerCreateInfo(const VkSamplerCreateInfo& info);
}