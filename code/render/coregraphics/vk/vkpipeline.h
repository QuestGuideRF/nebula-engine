#pragma once
#include "ids/idallocator.h"
namespace Vulkan
{
struct Pipeline
{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    CoreGraphics::PassId pass;
};
enum
{
    Pipeline_Object
};
extern Ids::IdAllocator<Pipeline> pipelineAllocator;
}