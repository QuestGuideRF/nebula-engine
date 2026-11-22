#pragma once
#include "vkloader.h"
#include "util/fixedarray.h"
#include "coregraphics/config.h"
namespace Vulkan
{
class VkSubContextHandler
{
public:
    VkSubContextHandler();
    ~VkSubContextHandler();
    struct TimelineSubmission
    {
        Util::Array<uint64_t> signalIndices;
        Util::Array<VkSemaphore> signalSemaphores;
        Util::Array<VkCommandBuffer> buffers;
        Util::Array<VkPipelineStageFlags> waitFlags;
        Util::Array<VkSemaphore> waitSemaphores;
        Util::Array<uint64_t> waitIndices;
    };
    struct TimelineSubmission2
    {
#if NEBULA_GRAPHICS_DEBUG
        const char* name = nullptr;
#endif
        CoreGraphics::QueueType queue;
        Util::Array<uint64_t, 16> signalIndices;
        Util::Array<VkSemaphore, 16> signalSemaphores;
        Util::Array<VkCommandBuffer, 16> buffers;
        Util::Array<VkPipelineStageFlags, 16> waitFlags;
        Util::Array<VkSemaphore, 16> waitSemaphores;
        Util::Array<uint64_t, 16> waitIndices;
    };
    struct SubmissionList
    {
        CoreGraphics::QueueType queue;
        Util::Array<TimelineSubmission2> submissions;
    };
    struct SparseBindSubmission
    {
        VkBindSparseInfo bindInfo;
        Util::Array<uint64_t, 16> signalIndices;
        Util::Array<VkSemaphore, 16> signalSemaphores;
        Util::FixedArray<VkSparseMemoryBind> opaqueMemoryBinds;
        Util::FixedArray<VkSparseImageMemoryBind> imageMemoryBinds;
        Util::FixedArray<VkSparseMemoryBind> bufferMemoryBinds;
        Util::Array<VkSparseBufferMemoryBindInfo, 16> bufferMemoryBindInfos;
        Util::Array<VkSparseImageOpaqueMemoryBindInfo, 16> imageOpaqueBindInfos;
        Util::Array<VkSparseImageMemoryBindInfo, 16> imageMemoryBindInfos;
        Util::Array<VkSemaphore, 16> waitSemaphores;
        Util::Array<uint64_t, 16> waitIndices;
    };
    void Setup(VkDevice dev, const Util::FixedArray<Util::Pair<uint, uint>> indexMap);
    void Discard();
    void SetToNextContext(const CoreGraphics::QueueType type);
    uint64_t AppendSubmissionTimeline(
        CoreGraphics::QueueType type
        , VkCommandBuffer cmds
        , Util::Array<CoreGraphics::SubmissionWaitEvent, 8> waitEvents
#if NEBULA_GRAPHICS_DEBUG
        , const char* name = nullptr
#endif
    );
    uint64_t AppendSubmissionTimeline(
        CoreGraphics::QueueType type
        , Util::Array<VkCommandBuffer, 16> cmds
        , Util::Array<CoreGraphics::SubmissionWaitEvent, 8> waitEvents
#if NEBULA_GRAPHICS_DEBUG
        , const char* name = nullptr
#endif
    );
    uint64_t GetNextTimelineIndex(CoreGraphics::QueueType type);
    uint64_t AppendSparseBind(CoreGraphics::QueueType type, const VkImage img, const Util::Array<VkSparseMemoryBind>& opaqueBinds, const Util::Array<VkSparseImageMemoryBind>& pageBinds);
    uint64_t AppendSparseBind(CoreGraphics::QueueType type, const VkBuffer buf, const Util::Array<VkSparseMemoryBind>& binds);
    void AppendPresentSignal(CoreGraphics::QueueType type, VkSemaphore sem);
    void FlushSubmissions(VkFence fence);
    void Wait(CoreGraphics::QueueType type, uint64_t index);
    bool Poll(CoreGraphics::QueueType type, uint64_t index);
    void FlushSparseBinds(VkFence fence);
    void InsertFence(CoreGraphics::QueueType type, VkFence fence);
    void WaitIdle(const CoreGraphics::QueueType type);
    VkQueue GetQueue(const CoreGraphics::QueueType type);
private:
    friend const VkQueue GetQueue(const CoreGraphics::QueueType type, const IndexT index);
    VkSemaphore GetSemaphore(const CoreGraphics::QueueType type);
    uint64_t GetSemaphoreId(const CoreGraphics::QueueType type);
    void IncrementSemaphoreId(const CoreGraphics::QueueType type);
    VkDevice device;
    Util::FixedArray<Util::Array<VkQueue>> queues;
    Util::FixedArray<uint> currentQueue;
    Util::FixedArray<Util::Array<VkSemaphore>> semaphores;
    Util::FixedArray<Util::Array<uint64_t>> semaphoreSubmissionIds;
    Util::FixedArray<Util::Array<TimelineSubmission2, 16>> submissions;
    Util::Array<SparseBindSubmission> sparseBindSubmissions;
    Threading::CriticalSection submissionLock;
};
}