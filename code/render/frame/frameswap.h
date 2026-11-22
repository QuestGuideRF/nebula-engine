#pragma once
#include "frameop.h"
namespace Frame
{
class FrameSwap : public FrameOp
{
public:
    FrameSwap();
    virtual ~FrameSwap();
    struct CompiledImpl : public FrameSwap::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
        CoreGraphics::TextureId from;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    CoreGraphics::TextureId from;
};
}