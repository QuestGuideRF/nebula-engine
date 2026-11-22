#pragma once
#include "graphics/graphicscontext.h"
#include "coregraphics/accelerationstructure.h"
#include "coregraphics/buffer.h"
#include "system_shaders/cluster_generate.h"
#include "materials/materialtemplates.h"
#include "materials/shaderconfig.h"
namespace Raytracing
{
struct RaytracingSetupSettings
{
    SizeT maxNumAllowedInstances;
};
enum ObjectType
{
    BRDFObject,
    BSDFObject,
    GLTFObject,
    ParticleObject
};
enum UpdateType
{
    Dynamic,
    Static
};
class RaytracingContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    RaytracingContext();
    ~RaytracingContext();
    static void Create(const RaytracingSetupSettings& settings);
    static void Discard();
    static void SetupModel(const Graphics::GraphicsEntityId id, CoreGraphics::BlasInstanceFlags flags, uchar mask);
    static void SetupMesh(
        const Graphics::GraphicsEntityId id
        , const UpdateType objectType
        , const CoreGraphics::VertexComponent::Format format
        , const CoreGraphics::IndexType::Code indexType
        , const CoreGraphics::VertexAlloc& vertices
        , const CoreGraphics::VertexAlloc& indices
        , const CoreGraphics::PrimitiveGroup& patchPrimGroup
        , const SizeT vertexOffsetStride
        , const SizeT patchVertexStride
        , const Util::Array<Math::mat4> transforms
        , const uint materialTableOffset
        , const MaterialTemplates::MaterialProperties shader
        , const CoreGraphics::VertexLayoutType vertexLayout
    );
    static void InvalidateBLAS(const Graphics::GraphicsEntityId id);
    static void ReconstructTopLevelAcceleration(const Graphics::FrameContext& ctx);
    static void UpdateTransforms(const Graphics::FrameContext& ctx);
    static void WaitForJobs(const Graphics::FrameContext& ctx);
    static void UpdateViewResources(const Ptr<Graphics::View>& view, const Graphics::FrameContext& ctx);
    static CoreGraphics::ResourceTableId GetLightGridResourceTable(IndexT bufferIndex);
    static CoreGraphics::TlasId GetTLAS();
    static CoreGraphics::BufferId GetObjectBindingBuffer();
    static CoreGraphics::ResourceTableId GetRaytracingTable(const IndexT bufferIndex);
#ifndef PUBLIC_DEBUG
    static void OnRenderDebug(uint32_t flags);
#endif
private:
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
    enum
    {
        Raytracing_Allocation,
        Raytracing_UpdateType,
        Raytracing_NumStructures,
        Raytracing_Blases
    };
    typedef Ids::IdAllocator<
        Memory::RangeAllocation,
        Raytracing::UpdateType,
        uint,
        Util::FixedArray<CoreGraphics::BlasId>
    > RaytracingAllocator;
    static RaytracingAllocator raytracingContextAllocator;
};
inline Graphics::ContextEntityId
RaytracingContext::Alloc()
{
    return raytracingContextAllocator.Alloc();
}
}