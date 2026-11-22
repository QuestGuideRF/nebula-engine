#pragma once
#include "graphics/graphicscontext.h"
namespace PostEffects
{
class DownsamplingContext : public Graphics::GraphicsContext
{
    __DeclarePluginContext()
public:
    DownsamplingContext();
    ~DownsamplingContext();
    static void Create();
    static void Discard();
    static void Setup();
    static void WindowResized(const CoreGraphics::WindowId windowId, SizeT width, SizeT height);
private:
    static Graphics::ContextEntityId Alloc() { return Graphics::InvalidContextEntityId; }
    static void Dealloc(Graphics::ContextEntityId id) {};
};
}