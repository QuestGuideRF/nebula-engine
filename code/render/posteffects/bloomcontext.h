#pragma once
#include "graphics/graphicscontext.h"
namespace PostEffects
{
class BloomContext : public Graphics::GraphicsContext
{
    __DeclarePluginContext();
public:
    BloomContext();
    virtual ~BloomContext();
    static void Create();
    static void Discard();
    static void Setup();
    static void WindowResized(const CoreGraphics::WindowId windowId, SizeT width, SizeT height);
private:
    static Graphics::ContextEntityId Alloc() { return Graphics::InvalidContextEntityId; }
    static void Dealloc(Graphics::ContextEntityId id) {};
};
}