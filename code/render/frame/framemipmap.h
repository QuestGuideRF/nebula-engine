#pragma once
#include "frameop.h"
namespace Frame
{
class FrameMipmap : public FrameOp
{
public:
    FrameMipmap();
    virtual ~FrameMipmap();
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        void Discard() override;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
        CoreGraphics::TextureId tex;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    CoreGraphics::TextureId tex;
};
}