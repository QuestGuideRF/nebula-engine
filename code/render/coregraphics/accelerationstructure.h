#pragma once
#include "mesh.h"
namespace CoreGraphics
{
enum class AccelerationStructureBuildFlags
{
    FastBuild = 0x1,
    FastTrace = 0x2,
    Dynamic = 0x4,
    Compact = 0x8,
    Small = 0x10,
};
__ImplementEnumBitOperators(AccelerationStructureBuildFlags);
__ImplementEnumComparisonOperators(AccelerationStructureBuildFlags);
struct BlasCreateInfo
{
    CoreGraphics::BufferId vbo, ibo;
    CoreGraphics::IndexType::Code indexType;
    CoreGraphics::VertexComponent::Format positionsFormat;
    uint64_t stride;
    uint64_t vertexOffset, indexOffset;
    CoreGraphics::PrimitiveGroup primGroup;
    AccelerationStructureBuildFlags flags;
};
ID_24_8_TYPE(BlasId);
_DECL_ACQUIRE_RELEASE(BlasId);
BlasId CreateBlas(const BlasCreateInfo& info);
void DestroyBlas(const BlasId blac);
enum BlasInstanceFlags
{
    NoFlags = 0x0,
    FaceCullingDisabled = 0x1,
    InvertFace = 0x2,
    ForceOpaque = 0x4,
    NoOpaque = 0x8
};
__ImplementEnumBitOperators(BlasInstanceFlags);
ID_24_8_TYPE(BlasInstanceId);
_DECL_ACQUIRE_RELEASE(BlasInstanceId);
struct BlasInstanceCreateInfo
{
    CoreGraphics::BlasId blas;
    Math::mat4 transform;
    uint instanceIndex;
    uchar mask;
    uint shaderOffset;
    BlasInstanceFlags flags;
};
BlasInstanceId CreateBlasInstance(const BlasInstanceCreateInfo& info);
void DestroyBlasInstance(const BlasInstanceId id);
void BlasInstanceUpdate(const BlasInstanceId id, const Math::mat4& transform, CoreGraphics::BufferId buf, uint offset);
void BlasInstanceUpdate(const BlasInstanceId id, CoreGraphics::BufferId buf, uint offset);
void BlasInstanceSetMask(const BlasInstanceId id, uint mask);
const SizeT BlasInstanceGetSize();
struct TlasCreateInfo
{
    SizeT numInstances;
    CoreGraphics::BufferId instanceBuffer;
    AccelerationStructureBuildFlags flags;
};
ID_24_8_TYPE(TlasId);
TlasId CreateTlas(const TlasCreateInfo& info);
void DestroyTlas(const TlasId tlas);
void TlasInitBuild(const TlasId tlas);
void TlasInitUpdate(const TlasId tlas);
}