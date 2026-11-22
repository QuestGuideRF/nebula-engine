#pragma once
#include "ids/idallocator.h"
#include "coregraphics/textureview.h"
namespace Vulkan
{
struct VkTextureViewLoadInfo
{
    VkDevice dev;
    CoreGraphics::TextureId tex;
    IndexT mip;
    SizeT numMips;
    IndexT layer;
    SizeT numLayers;
    CoreGraphics::PixelFormat::Code format;
    CoreGraphics::TextureSwizzle swizzle;
};
struct VkTextureViewRuntimeInfo
{
    VkImageView view;
};
enum
{
    TextureView_LoadInfo,
    TextureView_RuntimeInfo
};
typedef Ids::IdAllocator<
    VkTextureViewLoadInfo,
    VkTextureViewRuntimeInfo
> VkTextureViewAllocator;
extern VkTextureViewAllocator textureViewAllocator;
const VkImageView TextureViewGetVk(const CoreGraphics::TextureViewId id);
const VkDevice TextureViewGetVkDevice(const CoreGraphics::TextureViewId id);
}