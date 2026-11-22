#pragma once
#include "graphics/graphicscontext.h"
namespace PostEffects
{
class SSRContext : public Graphics::GraphicsContext
{
    __DeclarePluginContext();
public:
    SSRContext();
    virtual ~SSRContext();
    static void Create();
    static void Discard();
    static void Setup(const Ptr<Frame::FrameScript>& script);
    static void UpdateViewDependentResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
private:
    static Graphics::ContextEntityId Alloc() { return Graphics::InvalidContextEntityId; }
    static void Dealloc(Graphics::ContextEntityId id) {};
};
}