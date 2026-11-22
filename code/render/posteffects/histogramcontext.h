#pragma once
#include "graphics/graphicscontext.h"
namespace PostEffects
{
class HistogramContext : public Graphics::GraphicsContext
{
    __DeclarePluginContext();
public:
    HistogramContext();
    virtual ~HistogramContext();
    static void Create();
    static void Discard();
    static void SetWindow(const Math::float2 offset, Math::float2 size, int mip);
    static void Setup();
    static void UpdateViewResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
    static void UpdateConstants();
    static void WindowResized(const CoreGraphics::WindowId windowId, SizeT width, SizeT height);
private:
    static Graphics::ContextEntityId Alloc() { return Graphics::InvalidContextEntityId; }
    static void Dealloc(Graphics::ContextEntityId id) {};
};
}