#pragma once
#include "coregraphics/semaphore.h"
#include "ids/idallocator.h"
namespace Vulkan
{
enum
{
    Semaphore_Device,
    Semaphore_VkHandle,
    Semaphore_Type,
    Semaphore_LastIndex
};
typedef Ids::IdAllocator<
    VkDevice,
    VkSemaphore,
    CoreGraphics::SemaphoreType,
    uint64_t
> VkSemaphoreAllocator;
extern VkSemaphoreAllocator semaphoreAllocator;
VkSemaphore SemaphoreGetVk(const CoreGraphics::SemaphoreId& id);
}