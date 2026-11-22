#pragma once
#include "frameop.h"
#include "coregraphics/graphicsdevice.h"
namespace Frame
{
class FrameSubmission : public FrameOp
{
public:
    FrameSubmission();
    ~FrameSubmission();
    void OnWindowResized();
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        void Discard() override;
        Util::Array<FrameSubmission::CompiledImpl*> waitSubmissions;
        Util::Array<CoreGraphics::QueueType> waitQueues;
        CoreGraphics::CmdBufferPoolId commandBufferPool;
        CoreGraphics::QueueType queue;
        Util::Array<CoreGraphics::BarrierId>* resourceResetBarriers;
        Util::Array<FrameOp::Compiled*> compiled;
        CoreGraphics::SubmissionWaitEvent submissionId;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    virtual void Build(const BuildContext& ctx);
    Util::Array<FrameSubmission*> waitSubmissions;
    Util::Array<CoreGraphics::QueueType> waitQueues;
    CoreGraphics::CmdBufferPoolId commandBufferPool;
    CoreGraphics::QueueType queue;
    Util::Array<CoreGraphics::BarrierId>* resourceResetBarriers;
};
}