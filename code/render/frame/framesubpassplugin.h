#pragma once
#include "frameop.h"
namespace Frame
{
class FrameSubpassPlugin : public FrameOp
{
public:
    FrameSubpassPlugin();
    virtual ~FrameSubpassPlugin();
    void Setup();
    void Discard();
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        std::function<void(const CoreGraphics::CmdBufferId cmdBuf, IndexT, IndexT)> func;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    std::function<void(const CoreGraphics::CmdBufferId cmdBuf, IndexT, IndexT)> func;
private:
    void Build(
        Memory::ArenaAllocator<BIG_CHUNK>& allocator,
        Util::Array<FrameOp::Compiled*>& compiledOps,
        Util::Array<CoreGraphics::EventId>& events,
        Util::Array<CoreGraphics::BarrierId>& barriers,
        Util::Dictionary<CoreGraphics::BufferId, Util::Array<BufferDependency>>& rwBuffers,
        Util::Dictionary<CoreGraphics::TextureId, Util::Array<TextureDependency>>& textures
    );
};
}