#pragma once
#include "util/array.h"
#include "coregraphics/memory.h"
namespace Vulkan
{
CoreGraphics::Alloc AllocateMemory(const VkDevice dev, const VkImage& img, CoreGraphics::MemoryPoolType type);
CoreGraphics::Alloc AllocateMemory(const VkDevice dev, const VkBuffer& buf, CoreGraphics::MemoryPoolType type, uint alignment = 1);
CoreGraphics::Alloc AllocateMemory(const VkDevice dev, VkMemoryRequirements reqs, VkDeviceSize allocSize);
void Flush(const VkDevice dev, const CoreGraphics::Alloc& alloc, IndexT offset, SizeT size);
void Invalidate(const VkDevice dev, const CoreGraphics::Alloc& alloc, IndexT offset, SizeT size);
VkResult GetMemoryType(uint32_t bits, VkMemoryPropertyFlags flags, uint32_t& index);
}