#pragma once
#include "graphicscontext.h"
namespace Graphics
{
class EnvironmentContext : public GraphicsContext
{
    __DeclarePluginContext();
public:
    static void Create(const Graphics::GraphicsEntityId sun);
    static void OnBeforeFrame(const Graphics::FrameContext& ctx);
    static void RenderUI(const Graphics::FrameContext& ctx);
    static void SetFogColor(const Math::vec4& fogColor);
    static void SetFogDistances(const float nearFog, const float farFog);
    static void SetBloomColor(const Math::vec4& bloomColor);
    static void SetBloomIntensity(const float intensity);
    static void SetMaxLuminance(const float maxLuminance);
    static void SetNumEnvironmentMips(const int mips);
};
}