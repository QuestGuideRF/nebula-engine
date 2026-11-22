#pragma once
#include "frameop.h"
namespace Frame
{
class FrameBlit : public FrameOp
{
public:
    FrameBlit();
    virtual ~FrameBlit();
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        void Discard() override;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
        CoreGraphics::ImageBits fromBits, toBits;
        CoreGraphics::TextureId from, to;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    CoreGraphics::ImageBits fromBits, toBits;
    CoreGraphics::TextureId from, to;
};
}