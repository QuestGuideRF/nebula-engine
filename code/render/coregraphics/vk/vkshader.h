#pragma once
#include "core/refcounted.h"
#include "util/tupleutility.h"
#include "coregraphics/shader.h"
#include "coregraphics/sampler.h"
#include "coregraphics/resourcetable.h"
#include "vkshaderprogram.h"
namespace AnyFX
{
class ShaderEffect;
}
namespace Vulkan
{
const VkProgramReflectionInfo& ShaderGetProgramReflection(const CoreGraphics::ShaderProgramId shaderProgramId);
void ShaderSetup(
    VkDevice dev,
    const Util::StringAtom& name,
    AnyFX::ShaderEffect* effect,
    Util::FixedArray<CoreGraphics::ResourcePipelinePushConstantRange>& constantRange,
    Util::Set<CoreGraphics::SamplerId>& immutableSamplers,
    Util::FixedArray<Util::Pair<uint32_t, CoreGraphics::ResourceTableLayoutId>>& setLayouts,
    Util::Dictionary<uint32_t, uint32_t>& setLayoutMap,
    CoreGraphics::ResourcePipelineId& pipelineLayout,
    Util::Dictionary<Util::StringAtom, uint32_t>& resourceSlotMapping,
    Util::Dictionary<Util::StringAtom, IndexT>& constantBindings
);
void ShaderCleanup(
    VkDevice dev,
    Util::Set<CoreGraphics::SamplerId>& immutableSamplers,
    Util::FixedArray<Util::Pair<uint32_t, CoreGraphics::ResourceTableLayoutId>>& setLayouts,
    Util::Dictionary<Util::StringAtom, CoreGraphics::BufferId>& buffers,
    CoreGraphics::ResourcePipelineId& pipelineLayout
);
static Util::String VkShaderCreateSignature(const VkDescriptorSetLayoutBinding& bind);
extern Util::Dictionary<Util::StringAtom, VkDescriptorSetLayout> VkShaderLayoutCache;
extern Util::Dictionary<Util::StringAtom, VkPipelineLayout> VkShaderPipelineCache;
extern Util::Dictionary<Util::StringAtom, VkDescriptorSet> VkShaderDescriptorSetCache;
struct DescriptorSetBinding
{
    VkDescriptorSet set;
    VkPipelineLayout layout;
    IndexT slot;
};
struct BufferMapping
{
    uint32_t index;
    uint32_t offset;
};
typedef Util::Dictionary<Util::StringAtom, CoreGraphics::BufferId> UniformBufferMap;
typedef Util::Dictionary<uint32_t, Util::Array<CoreGraphics::BufferId>> UniformBufferGroupMap;
typedef Util::Dictionary<CoreGraphics::ShaderFeature::Mask, CoreGraphics::ShaderProgramId> ProgramMap;
struct VkShaderRuntimeInfo
{
    CoreGraphics::ShaderFeature::Mask activeMask;
    CoreGraphics::ShaderProgramId activeShaderProgram;
    ProgramMap programMap;
};
struct VkShaderSetupInfo
{
    VkDevice dev;
    Resources::ResourceName name;
    CoreGraphics::ShaderIdentifier::Code id;
    UniformBufferMap uniformBufferMap;
    UniformBufferGroupMap uniformBufferGroupMap;
    CoreGraphics::ResourcePipelineId pipelineLayout;
    Util::FixedArray<CoreGraphics::ResourcePipelinePushConstantRange> constantRangeLayout;
    Util::Set<CoreGraphics::SamplerId> immutableSamplers;
    Util::Dictionary<Util::StringAtom, uint32_t> resourceIndexMap;
    Util::Dictionary<Util::StringAtom, IndexT> constantBindings;
    Util::FixedArray<Util::Pair<uint32_t, CoreGraphics::ResourceTableLayoutId>> descriptorSetLayouts;
    Util::Dictionary<uint32_t, uint32_t> descriptorSetLayoutMap;
};
struct VkReflectionInfo
{
    struct UniformBuffer
    {
        uint32_t set;
        uint32_t binding;
        uint32_t byteSize;
        Util::StringAtom name;
    };
    Util::Array<Util::Array<UniformBuffer>> uniformBuffersPerSet;
    Util::Dictionary<Util::StringAtom, UniformBuffer> uniformBuffersByName;
    Util::Array<UniformBuffer> uniformBuffers;
    struct Variable
    {
        AnyFX::VariableType type;
        Util::StringAtom name;
        Util::StringAtom blockName;
        uint32_t blockSet;
        uint32_t blockBinding;
    };
    Util::Dictionary<Util::StringAtom, Variable> variablesByName;
    Util::Array<Variable> variables;
    Util::Array<uint64_t> uniformBuffersMask;
};
enum
{
    Shader_ReflectionInfo,
    Shader_SetupInfo,
    Shader_RuntimeInfo,
    Shader_Programs,
};
typedef Ids::IdAllocator<
    VkReflectionInfo,
    VkShaderSetupInfo,
    VkShaderRuntimeInfo,
    Util::Array<CoreGraphics::ShaderProgramId>
> ShaderAllocator;
extern ShaderAllocator shaderAlloc;
}