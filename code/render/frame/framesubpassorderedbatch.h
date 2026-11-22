#pragma once
#include "frameop.h"
namespace Frame
{
class FrameSubpassOrderedBatch : public FrameOp
{
public:
    FrameSubpassOrderedBatch();
    virtual ~FrameSubpassOrderedBatch();
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        MaterialTemplates::BatchGroup batch;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    MaterialTemplates::BatchGroup batch;
};
}