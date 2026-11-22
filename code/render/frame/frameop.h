#pragma once
#include "core/refcounted.h"
#include "util/stringatom.h"
#include "util/tupleutility.h"
#include "coregraphics/config.h"
#include "coregraphics/barrier.h"
#include "coregraphics/event.h"
#include "coregraphics/semaphore.h"
#include "memory/arenaallocator.h"
#include "coregraphics/commandbuffer.h"
#include "coregraphics/pass.h"
namespace Frame
{
enum class DependencyIntent
{
    Read,
    Write
};
class FrameOp
{
public:
    FrameOp();
    virtual ~FrameOp();
    virtual void Discard();
    void SetName(const Util::StringAtom& name);
    const Util::StringAtom& GetName() const;
    void SetEnabled(bool b);
    const bool GetEnabled() const;
    void AddChild(FrameOp* op);
    const Util::Array<Frame::FrameOp*>& GetChildren();
    virtual void OnWindowResized();
    CoreGraphics::BarrierDomain domain;
    CoreGraphics::QueueType queue;
    struct BufferSubresourceDep
    {
        Util::StringAtom name;
        CoreGraphics::PipelineStage stage;
        CoreGraphics::BufferSubresourceInfo subres;
    };
    struct TextureSubresourceDep
    {
        Util::StringAtom name;
        CoreGraphics::PipelineStage stage;
        CoreGraphics::TextureSubresourceInfo subres;
    };
    Util::Dictionary<CoreGraphics::TextureId, Util::Tuple<Util::StringAtom, CoreGraphics::PipelineStage, CoreGraphics::TextureSubresourceInfo>> textureDeps;
    Util::Dictionary<CoreGraphics::BufferId, Util::Tuple<Util::StringAtom, CoreGraphics::PipelineStage, CoreGraphics::BufferSubresourceInfo>> bufferDeps;
    Util::Dictionary<const CoreGraphics::TextureId*, Util::Tuple<Util::StringAtom, CoreGraphics::PipelineStage, CoreGraphics::TextureSubresourceInfo>> textureDepRefs;
    Util::Dictionary<const CoreGraphics::BufferId*, Util::Tuple<Util::StringAtom, CoreGraphics::PipelineStage, CoreGraphics::BufferSubresourceInfo>> bufferDepRefs;
protected:
    friend class FrameScriptLoader;
    friend class FrameScript;
    friend class FramePass;
    friend class FrameSubpass;
    friend class FrameSubmission;
    friend class FrameSubgraph;
    struct Compiled
    {
        Compiled()
        {
        }
        virtual void Run(const CoreGraphics::CmdBufferId cmdBuf, const IndexT frameIndex, const IndexT bufferIndex) = 0;
        virtual void Discard();
        virtual void SetupConstants(const IndexT bufferIndex);
        virtual void QueuePreSync(const CoreGraphics::CmdBufferId cmdBuf);
        Util::Array<CoreGraphics::BarrierId> barriers;
        CoreGraphics::QueueType queue;
    };
    struct TextureDependency
    {
        CoreGraphics::PipelineStage stage;
        DependencyIntent intent;
        CoreGraphics::TextureSubresourceInfo subres;
        CoreGraphics::QueueType queue;
    };
    struct BufferDependency
    {
        CoreGraphics::PipelineStage stage;
        DependencyIntent intent;
        CoreGraphics::BufferSubresourceInfo subres;
        CoreGraphics::QueueType queue;
    };
    static void AnalyzeAndSetupTextureBarriers(
        struct FrameOp::Compiled* op,
        CoreGraphics::TextureId tex,
        const Util::StringAtom& textureName,
        DependencyIntent readOrWrite,
        CoreGraphics::PipelineStage stage,
        CoreGraphics::BarrierDomain domain,
        const CoreGraphics::TextureSubresourceInfo& subres,
        CoreGraphics::QueueType fromQueue,
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::BarrierCreateInfo>& barriers,
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::EventCreateInfo>& waitEvents,
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, struct FrameOp::Compiled*>& signalEvents,
        Util::Array<FrameOp::TextureDependency>& renderTextureDependencies);
    static void AnalyzeAndSetupBufferBarriers(
        struct FrameOp::Compiled* op,
        CoreGraphics::BufferId buf,
        const Util::StringAtom& bufferName,
        DependencyIntent readOrWrite,
        CoreGraphics::PipelineStage stage,
        CoreGraphics::BarrierDomain domain,
        const CoreGraphics::BufferSubresourceInfo& subres,
        CoreGraphics::QueueType fromQueue,
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::BarrierCreateInfo>& barriers,
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, CoreGraphics::EventCreateInfo>& waitEvents,
        Util::Dictionary<Util::Tuple<CoreGraphics::PipelineStage, CoreGraphics::PipelineStage>, struct FrameOp::Compiled*>& signalEvents,
        Util::Array<FrameOp::BufferDependency>& bufferDependencies);
    virtual Compiled* AllocCompiled(Memory::ArenaAllocator<BIG_CHUNK>& allocator) = 0;
    struct BuildContext
    {
        CoreGraphics::PassId currentPass;
        uint subpass;
        Memory::ArenaAllocator<BIG_CHUNK>& allocator;
        Util::Array<FrameOp::Compiled*>& compiledOps;
        Util::Array<CoreGraphics::EventId>& events;
        Util::Array<CoreGraphics::BarrierId>& barriers;
        Util::Dictionary<CoreGraphics::BufferId, Util::Array<BufferDependency>>& buffers;
        Util::Dictionary<CoreGraphics::TextureId, Util::Array<TextureDependency>>& textures;
    };
    virtual void Build(const BuildContext& ctx);
    void SetupSynchronization(
        Memory::ArenaAllocator<BIG_CHUNK>& allocator,
        Util::Array<CoreGraphics::EventId>& events,
        Util::Array<CoreGraphics::BarrierId>& barriers,
        Util::Dictionary<CoreGraphics::BufferId, Util::Array<BufferDependency>>& buffers,
        Util::Dictionary<CoreGraphics::TextureId, Util::Array<TextureDependency>>& textures);
    Util::Array<FrameOp*> children;
    Util::Dictionary<Util::StringAtom, FrameOp*> childrenByName;
    Compiled* compiled;
    Util::StringAtom name;
    IndexT index;
    bool enabled;
};
inline void
FrameOp::SetName(const Util::StringAtom& name)
{
    this->name = name;
}
inline const Util::StringAtom&
FrameOp::GetName() const
{
    return this->name;
}
inline void
FrameOp::SetEnabled(bool b)
{
    this->enabled = b;
}
inline const bool
FrameOp::GetEnabled() const
{
    return this->enabled;
}
inline void
FrameOp::AddChild(FrameOp* op)
{
    this->children.Append(op);
    this->childrenByName.Add(op->GetName(), op);
}
inline const Util::Array<Frame::FrameOp*>&
FrameOp::GetChildren()
{
    return this->children;
}
}