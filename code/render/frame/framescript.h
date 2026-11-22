#pragma once
#include "core/refcounted.h"
#include "coregraphics/texture.h"
#include "coregraphics/buffer.h"
#include "coregraphics/event.h"
#include "coregraphics/shader.h"
#include "frame/frameop.h"
#include "frame/framepass.h"
#include "memory/arenaallocator.h"
#include "threading/event.h"
namespace Graphics
{
    class View;
    class GraphicsServer;
}
namespace CoreGraphics
{
    class DrawThread;
}
namespace Frame
{
class FrameScript : public Core::RefCounted
{
    __DeclareClass(FrameScript);
public:
    FrameScript();
    virtual ~FrameScript();
    Memory::ArenaAllocator<BIG_CHUNK>& GetAllocator();
    void SetResourceName(const Resources::ResourceName& name);
    const Resources::ResourceName& GetResourceName() const;
    void AddOp(Frame::FrameOp* op);
    Frame::FrameOp* GetOp(const Util::String& search);
    void AddTexture(const Util::StringAtom& name, const CoreGraphics::TextureId tex);
    const CoreGraphics::TextureId GetTexture(const Util::StringAtom& name);
    const Util::Dictionary<Util::StringAtom, CoreGraphics::TextureId>& GetTextures() const;
    void AddBuffer(const Util::StringAtom& name, const CoreGraphics::BufferId buf);
    const CoreGraphics::BufferId GetBuffer(const Util::StringAtom& name);
    void Setup();
    void Discard();
    void Run(const IndexT frameIndex, const IndexT bufferIndex);
    void Build();
private:
    friend class FrameScriptLoader;
    friend class FrameServer;
    friend class Graphics::View;
    friend class Graphics::GraphicsServer;
    void Cleanup();
    void OnWindowResized();
    CoreGraphics::WindowId window;
    Memory::ArenaAllocator<BIG_CHUNK> allocator;
    Util::Array<CoreGraphics::EventId> events;
    Util::Array<CoreGraphics::BarrierId> barriers;
    Memory::ArenaAllocator<BIG_CHUNK> buildAllocator;
    Resources::ResourceName resId;
    Util::Array<CoreGraphics::BufferId> buffers;
    Util::Dictionary<Util::StringAtom, CoreGraphics::BufferId> buffersByName;
    Util::Array<CoreGraphics::TextureId> textures;
    Util::Dictionary<Util::StringAtom, CoreGraphics::TextureId> texturesByName;
    Util::Array<Frame::FrameOp*> ops;
    Util::Dictionary<Util::StringAtom, Frame::FrameOp*> opsByName;
    Util::Array<Frame::FrameOp::Compiled*> compiled;
    Util::Array<CoreGraphics::BarrierId> resourceResetBarriers;
    IndexT frameOpCounter;
    bool subScript;
};
inline void
FrameScript::SetResourceName(const Resources::ResourceName& name)
{
    this->resId = name;
}
inline const Resources::ResourceName&
FrameScript::GetResourceName() const
{
    return this->resId;
}
inline Memory::ArenaAllocator<BIG_CHUNK>&
FrameScript::GetAllocator()
{
    return this->allocator;
}
inline const CoreGraphics::TextureId
FrameScript::GetTexture(const Util::StringAtom& name)
{
    return this->texturesByName[name];
}
inline const Util::Dictionary<Util::StringAtom, CoreGraphics::TextureId>&
FrameScript::GetTextures() const
{
    return this->texturesByName;
}
inline const CoreGraphics::BufferId
FrameScript::GetBuffer(const Util::StringAtom& name)
{
    return this->buffersByName[name];
}
}