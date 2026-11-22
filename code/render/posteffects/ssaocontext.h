#pragma once
namespace PostEffects
{
class SSAOContext : public Graphics::GraphicsContext
{
    __DeclarePluginContext();
public:
    SSAOContext();
    virtual ~SSAOContext();
    static void Create();
    static void Discard();
    static void Setup();
    static void UpdateViewDependentResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
    static void WindowResized(const CoreGraphics::WindowId id, SizeT width, SizeT height);
private:
    static Graphics::ContextEntityId Alloc() { return Graphics::InvalidContextEntityId; }
    static void Dealloc(Graphics::ContextEntityId id) {};
};
}