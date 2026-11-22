#pragma once
#include "frameop.h"
#include "math/rectangle.h"
namespace Frame
{
class FrameSubpass : public FrameOp
{
public:
    FrameSubpass();
    virtual ~FrameSubpass();
    void Discard() override;
    void OnWindowResized() override;
    void AddViewport(const Math::rectangle<int>& rect);
    void AddScissor(const Math::rectangle<int>& rect);
    struct CompiledImpl : public FrameOp::Compiled
    {
        void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) override;
        void Discard() override;
        Util::Array<Frame::FrameOp::Compiled*> ops;
        Util::Array<Math::rectangle<int>> viewports;
        Util::Array<Math::rectangle<int>> scissors;
#if NEBULA_GRAPHICS_DEBUG
        Util::StringAtom name;
#endif
    };
    FrameOp::Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator) override;
protected:
    friend class FramePass;
    void Build(const BuildContext& ctx) override;
private:
    friend class FrameScriptLoader;
};
}