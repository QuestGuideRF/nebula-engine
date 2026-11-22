#pragma once
#include "frameop.h"
#include "util/stringatom.h"
namespace Frame
{
class FrameSubgraph : public FrameOp
{
public:
    FrameSubgraph();
    virtual ~FrameSubgraph();
    void OnWindowResized() override;
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        void Discard() override;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
        Util::Array<FrameOp::Compiled*> subgraphOps;
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator) override;
private:
    void Build(const BuildContext& ctx) override;
};
extern void AddSubgraph(const Util::StringAtom name, const Util::Array<FrameOp*> ops);
const Util::Array<FrameOp*> GetSubgraph(const Util::StringAtom name);
extern Util::Dictionary<Util::StringAtom, Util::Array<FrameOp*>> nameToSubgraph;
}