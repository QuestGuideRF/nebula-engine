#pragma once
#include "frameop.h"
#include "coregraphics/shader.h"
#include "system_shaders/msaa/msaaresolvedepth4.h"
namespace Frame
{
class FrameResolve : public FrameOp
{
public:
    FrameResolve();
    ~FrameResolve();
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        void SetupConstants(const IndexT bufferIndex) override;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
        CoreGraphics::ImageBits fromBits, toBits;
        CoreGraphics::TextureId from, to;
        Msaaresolvedepth4::ResolveBlock constants;
        bool shaderResolve;
        Math::uint2 dispatchDims;
        CoreGraphics::ShaderProgramId program;
        Util::FixedArray<CoreGraphics::ResourceTableId> resourceTables;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator);
    CoreGraphics::ImageBits fromBits, toBits;
    CoreGraphics::TextureId from, to;
};
}