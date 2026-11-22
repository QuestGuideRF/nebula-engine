#pragma once
#include "coregraphics/event.h"
#include "ids/idallocator.h"
#include "util/stringatom.h"
namespace Vulkan
{
static const SizeT EventMaxNumBarriers = 16;
struct VkEventInfo
{
    Util::StringAtom name;
    VkEvent event;
    VkPipelineStageFlags leftDependency;
    VkPipelineStageFlags rightDependency;
    VkImageMemoryBarrier imageBarriers[EventMaxNumBarriers];
    uint32_t numImageBarriers;
    VkBufferMemoryBarrier bufferBarriers[EventMaxNumBarriers];
    uint32_t numBufferBarriers;
    VkMemoryBarrier memoryBarriers[EventMaxNumBarriers];
    uint32_t numMemoryBarriers;
};
enum
{
    Event_Device
    , Event_Info
};
typedef Ids::IdAllocator<
    VkDevice
    , VkEventInfo
> VkEventAllocator;
extern VkEventAllocator eventAllocator;
const VkEventInfo& EventGetVk(const CoreGraphics::EventId id);
}