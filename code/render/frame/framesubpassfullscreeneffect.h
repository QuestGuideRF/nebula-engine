#pragma once
#include "frameop.h"
#include "renderutil/drawfullscreenquad.h"
#include "coregraphics/shader.h"
#include "coregraphics/resourcetable.h"
namespace Frame
{
class FrameSubpassFullscreenEffect : public FrameOp
{
public:
    FrameSubpassFullscreenEffect();
    virtual ~FrameSubpassFullscreenEffect();
    void Setup();
    void Discard() override;
    void OnWindowResized() override;
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        CoreGraphics::ShaderProgramId program;
        CoreGraphics::ResourceTableId resourceTable;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator) override;
    Util::Dictionary<Util::StringAtom, CoreGraphics::BufferId> constantBuffers;
    Util::Array<Util::Tuple<IndexT, CoreGraphics::BufferId, CoreGraphics::TextureId>> textures;
    CoreGraphics::ResourceTableId resourceTable;
    CoreGraphics::ShaderId shader;
    CoreGraphics::ShaderProgramId program;
    CoreGraphics::TextureId tex;
};
}