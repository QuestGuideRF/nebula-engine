#pragma once
#include "util/fixedarray.h"
#include "coregraphics/config.h"
#include "coregraphics/barrier.h"
#include <vulkan/vulkan.h>
namespace Vulkan
{
struct VkDeferredCommand
{
    VkDeferredCommand();
    ~VkDeferredCommand();
    enum DelegateType
    {
        FreeCmdBuffers,
        FreeMemory,
        FreeBuffer,
        FreeImage,
        __RunAfterFence,
        BindDescriptorSets,
        UpdateBuffer,
        UpdateImage,
        DestroyPipeline,
        ClearColorImage,
        ClearDepthStencilImage,
        ImageOwnershipChange,
        ImageLayoutTransition
    };
    struct Delegate
    {
        DelegateType type;
        CoreGraphics::QueueType queue;
        VkFence fence;
        union
        {
            struct
            {
                VkCommandBuffer buffers[64];
                VkCommandPool pool;
                uint32_t numBuffers;
            } cmdbufferfree;
            struct
            {
                void* data;
            } memory;
            struct
            {
                VkBuffer buf;
                VkDeviceMemory mem;
            } buffer;
            struct
            {
                VkImage img;
                VkDeviceMemory mem;
            } image;
            struct
            {
                VkBuffer buf;
                VkDeviceSize offset;
                VkDeviceSize size;
                uint32_t* data;
            } bufferUpd;
            struct
            {
                VkImage img;
                VkImageCreateInfo info;
                uint32_t mip;
                uint32_t face;
                VkDeviceSize size;
                uint32_t* data;
            } imageUpd;
            struct
            {
                VkPipeline pipeline;
            } pipelineDestroy;
            struct
            {
                VkPipelineStageFlags left;
                VkPipelineStageFlags right;
                VkImageMemoryBarrier barrier;
            } imgBarrier;
            struct
            {
                VkImage img;
                VkImageLayout layout;
                VkClearColorValue clearValue;
                VkImageSubresourceRange region;
            } imgColorClear;
            struct
            {
                VkImage img;
                VkImageLayout layout;
                VkClearDepthStencilValue clearValue;
                VkImageSubresourceRange region;
            } imgDepthStencilClear;
            struct
            {
                VkPipelineStageFlags left;
                VkPipelineStageFlags right;
                VkImageMemoryBarrier barrier;
            } imgOwnerChange;
            struct
            {
                VkPipelineBindPoint type;
                uint32_t baseSet;
                uint32_t numSets;
                const VkDescriptorSet* sets;
                uint32_t numOffsets;
                const uint32_t* offsets;
            } descSetBind;
        };
    } del;
    VkDevice dev;
    void RunDelegate();
};
}