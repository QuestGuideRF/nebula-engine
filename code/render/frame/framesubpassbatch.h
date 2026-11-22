#pragma once
#include "frameop.h"
#include "materials/materialtemplates.h"
#include "graphics/graphicsentity.h"
namespace Frame
{
class FrameSubpassBatch : public FrameOp
{
public:
    FrameSubpassBatch();
    virtual ~FrameSubpassBatch();
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        MaterialTemplates::BatchGroup batch;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    MaterialTemplates::BatchGroup batch;
    static void DrawBatch(const CoreGraphics::CmdBufferId cmdBuf, MaterialTemplates::BatchGroup batch, const Graphics::GraphicsEntityId id, const IndexT bufferIndex);
    static void DrawBatch(const CoreGraphics::CmdBufferId cmdBuf, MaterialTemplates::BatchGroup batch, const Graphics::GraphicsEntityId id, const SizeT numInstances, const IndexT baseInstance, const IndexT bufferIndex);
};
}