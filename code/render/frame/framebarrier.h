#pragma once
#include "frameop.h"
#include "coregraphics/barrier.h"
namespace Frame
{
class FrameBarrier : public FrameOp
{
public:
    FrameBarrier();
    virtual ~FrameBarrier();
    struct CompiledImpl : public FrameOp::Compiled
    {
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
    };
private:
    virtual FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
};
}