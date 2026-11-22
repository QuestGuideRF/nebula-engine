#pragma once
#include "vkdeferredcommand.h"
#include "core/singleton.h"
#include "util/dictionary.h"
namespace Vulkan
{
class VkScheduler
{
    __DeclareSingleton(VkScheduler);
public:
    enum CommandPass
    {
        OnBeginFrame,
        OnEndFrame,
        OnBeginPass,
        OnNextSubpass,
        OnEndPass,
        OnMainTransferSubmitted,
        OnMainDrawSubmitted,
        OnMainComputeSubmitted,
        OnBeginTransferThread,
        OnBeginDrawThread,
        OnBeginComputeThread,
        OnTransferThreadsSubmitted,
        OnDrawThreadsSubmitted,
        OnComputeThreadsSubmitted,
        OnHandleTransferFences,
        OnHandleDrawFences,
        OnHandleComputeFences,
        OnHandleSparseFences,
        OnHandleFences,
        OnBindGraphicsPipeline,
        OnBindComputePipeline,
        NumCommandPasses
    };
    VkScheduler();
    virtual ~VkScheduler();
    void PushImageLayoutTransition(CoreGraphicsQueueType queue, CoreGraphics::BarrierStage left, CoreGraphics::BarrierStage right, VkImageMemoryBarrier barrier);
    void PushImageOwnershipChange(CoreGraphicsQueueType queue, CoreGraphics::BarrierStage left, CoreGraphics::BarrierStage right, VkImageMemoryBarrier barrier);
    void PushImageColorClear(const VkImage& image, const CoreGraphicsQueueType queue, VkImageLayout layout, VkClearColorValue clearValue, VkImageSubresourceRange subres);
    void PushImageDepthStencilClear(const VkImage& image, const CoreGraphicsQueueType queue, VkImageLayout layout, VkClearDepthStencilValue clearValue, VkImageSubresourceRange subres);
    void PushImageUpdate(const VkImage& img, const VkImageCreateInfo& info, uint32_t mip, uint32_t face, VkDeviceSize size, uint32_t* data);
    void ExecuteCommandPass(const CommandPass& pass);
    void SetDevice(const VkDevice dev);
    void Begin();
    void EndTransfers();
    void EndDraws();
    void EndComputes();
    void Discard();
private:
    void PushCommand(const VkDeferredCommand& cmd, const CommandPass& pass);
    friend class VkUtilities;
    friend void BindDescriptorsGraphics(const VkDescriptorSet* descriptors, uint32_t baseSet, uint32_t setCount, const uint32_t* offsets, uint32_t offsetCount, bool shared);
    friend void BindDescriptorsCompute(const VkDescriptorSet* descriptors, uint32_t baseSet, uint32_t setCount, const uint32_t* offsets, uint32_t offsetCount);
    friend void EndDrawThreads();
    friend void EndDrawSubpass();
    bool putTransferFenceThisFrame;
    bool putSparseFenceThisFrame;
    bool putComputeFenceThisFrame;
    bool putDrawFenceThisFrame;
    VkDevice dev;
    Util::FixedArray<Util::Array<VkDeferredCommand>> commands;
    Util::Dictionary<VkFence, Util::Array<VkDeferredCommand>> transferFenceCommands;
    Util::Dictionary<VkFence, Util::Array<VkDeferredCommand>> drawFenceCommands;
    Util::Dictionary<VkFence, Util::Array<VkDeferredCommand>> computeFenceCommands;
    Util::Dictionary<VkFence, Util::Array<VkDeferredCommand>> sparseFenceCommands;
};
inline void
VkScheduler::SetDevice(const VkDevice dev)
{
    this->dev = dev;
}
}