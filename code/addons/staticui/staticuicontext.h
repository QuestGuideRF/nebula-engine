#pragma once
#include "graphics/graphicscontext.h"
#include "ultralight/ultralightrenderer.h"
namespace StaticUI
{
class StaticUIContext : public Graphics::GraphicsContext
{
    __DeclarePluginContext();
public:
    StaticUIContext();
    virtual ~StaticUIContext();
    static void Create();
    static void Discard();
    static void Render();
    static void RegisterFunction(const char* name, std::function<void()> func);
};
}