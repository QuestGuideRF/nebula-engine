#pragma once
#include "ids/id.h"
#include "ids/idpool.h"
#include "resources/resourceid.h"
#include "coregraphics/shaderfeature.h"
#include "coregraphics/buffer.h"
#include "util/variant.h"
#include "util/dictionary.h"
namespace AnyFX
{
class ShaderEffect;
}
namespace CoreGraphics
{
struct TextureId;
struct ShaderRWBufferId;
struct ResourceTableId;
struct ResourceTableLayoutId;
struct ResourcePipelineId;
struct ResourceTableSet;
RESOURCE_ID_TYPE(ShaderId);
ID_24_8_24_8_NAMED_TYPE(ShaderProgramId, programId, programGeneration, shaderId, shaderGeneration, program, shader);
ID_32_TYPE(DerivativeStateId);
struct ShaderCreateInfo
{
    Resources::ResourceName name;
    AnyFX::ShaderEffect* effect;
};
enum ShaderConstantType
{
    UnknownVariableType,
    IntVariableType,
    FloatVariableType,
    VectorVariableType,
    Vector2VariableType,
    MatrixVariableType,
    BoolVariableType,
    TextureVariableType,
    SamplerVariableType,
    ConstantBufferVariableType,
    ImageReadWriteVariableType,
    BufferReadWriteVariableType,
    TextureHandleType,
    ImageHandleType,
    SamplerHandleType
};
const Util::String ConstantTypeToString(const ShaderConstantType& type);
const ShaderId CreateShader(const ShaderCreateInfo& info);
void DestroyShader(const ShaderId id);
void ReloadShader(const ShaderId id, const AnyFX::ShaderEffect* effect);
const ShaderId ShaderGet(const Resources::ResourceName& name);
CoreGraphics::ShaderFeature::Mask ShaderFeatureMask(const Util::String& mask);
const ResourceTableId ShaderCreateResourceTable(const ShaderId id, const IndexT group, const uint overallocationSize = 1, const char* name = nullptr);
ResourceTableSet ShaderCreateResourceTableSet(const ShaderId id, const IndexT group, const uint overallocationSize = 1, const char* name = nullptr);
const bool ShaderHasResourceTable(const ShaderId id, const IndexT group);
const BufferId ShaderCreateConstantBuffer(const ShaderId id, const Util::StringAtom& name, BufferAccessMode mode = BufferAccessMode::HostCached);
const BufferId ShaderCreateConstantBuffer(const ShaderId id, const IndexT cbIndex, BufferAccessMode mode = BufferAccessMode::HostCached);
const BufferId ShaderCreateConstantBuffer(const ShaderId id, const IndexT group, const IndexT cbIndex, BufferAccessMode mode = BufferAccessMode::HostCached);
const uint ShaderCalculateConstantBufferIndex(const uint64_t bindingMask, const IndexT slot);
const Resources::ResourceName ShaderGetName(const ShaderId id);
const SizeT ShaderGetConstantCount(const ShaderId id);
const ShaderConstantType ShaderGetConstantType(const ShaderId id, const IndexT i);
const ShaderConstantType ShaderGetConstantType(const ShaderId id, const Util::StringAtom& name);
const Util::StringAtom ShaderGetConstantName(const ShaderId id, const IndexT i);
const IndexT ShaderGetConstantBinding(const ShaderId id, const Util::StringAtom& name);
const IndexT ShaderGetConstantBinding(const ShaderId id, const IndexT cIndex);
const Util::StringAtom ShaderGetConstantBlockName(const ShaderId id, const Util::StringAtom& name);
const Util::StringAtom ShaderGetConstantBlockName(const ShaderId id, const IndexT cIndex);
const SizeT ShaderGetConstantBindingsCount(const ShaderId id);
const IndexT ShaderGetConstantGroup(const ShaderId id, const Util::StringAtom& name);
const IndexT ShaderGetConstantSlot(const ShaderId id, const Util::StringAtom& name);
CoreGraphics::ResourceTableLayoutId ShaderGetResourceTableLayout(const ShaderId id, const IndexT group);
CoreGraphics::ResourcePipelineId ShaderGetResourcePipeline(const ShaderId id);
const SizeT ShaderGetConstantBufferCount(const ShaderId id);
const SizeT ShaderGetConstantBufferSize(const ShaderId id, const IndexT i);
const Util::StringAtom ShaderGetConstantBufferName(const ShaderId id, const IndexT i);
const IndexT ShaderGetConstantBufferResourceSlot(const ShaderId id, const IndexT i);
const IndexT ShaderGetConstantBufferResourceGroup(const ShaderId id, const IndexT i);
const uint64_t ShaderGetConstantBufferBindingMask(const ShaderId id, const IndexT group);
const uint64_t ShaderGetConstantBufferSize(const ShaderId id, const IndexT group, const IndexT i);
const IndexT ShaderGetResourceSlot(const ShaderId id, const Util::StringAtom& name);
const Util::Dictionary<ShaderFeature::Mask, ShaderProgramId>& ShaderGetPrograms(const ShaderId id);
const Util::StringAtom ShaderProgramGetName(const ShaderProgramId id);
const CoreGraphics::ShaderProgramId ShaderGetProgram(const ShaderId id, const CoreGraphics::ShaderFeature::Mask mask);
union RayTracingBits
{
    struct
    {
        bool hasGen : 1;
        bool hasCallable : 1;
        bool hasAnyHit : 1;
        bool hasClosestHit : 1;
        bool hasIntersect : 1;
        bool hasMiss : 1;
    } bitField;
    uint bits;
};
RayTracingBits ShaderProgramGetRaytracingBits(const ShaderProgramId id);
}