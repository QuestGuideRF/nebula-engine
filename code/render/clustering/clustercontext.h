#pragma once
#include "graphics/graphicscontext.h"
#include "coregraphics/buffer.h"
#include "coregraphics/window.h"
#include <array>
#include "system_shaders/cluster_generate.h"
namespace Clustering
{
class ClusterContext : public Graphics::GraphicsContext
{
    __DeclarePluginContext();
public:
    ClusterContext();
    virtual ~ClusterContext();
    static void Create(float ZNear, float ZFar, const CoreGraphics::WindowId window);
    static const SizeT GetNumClusters();
    static const std::array<SizeT, 3> GetClusterDimensions();
    static void UpdateResources(const Graphics::FrameContext& ctx);
#ifndef PUBLIC_BUILD
    static void OnRenderDebug(uint32_t flags);
#endif
    static void WindowResized(const CoreGraphics::WindowId id, SizeT width, SizeT height);
    static const CoreGraphics::BufferId* GetClusterBuffer();
private:
    static void UpdateClusters();
};
}