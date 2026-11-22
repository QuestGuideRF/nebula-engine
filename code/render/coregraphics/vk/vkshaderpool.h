#pragma once
#include "core/refcounted.h"
#include "coregraphics/config.h"
#include "resources/resourcestreamcache.h"
#include "util/dictionary.h"
#include "coregraphics/shaderfeature.h"
#include "lowlevel/vk/vkprogram.h"
#include "coregraphics/shader.h"
#include "vkloader.h"
#include "coregraphics/buffer.h"
#include "coregraphics/shaderidentifier.h"
#include "coregraphics/resourcetable.h"
#include "vkshaderprogram.h"
#include "vktexture.h"
#include "vkshader.h"
namespace CoreGraphics
{
    void SetShaderProgram(const CoreGraphics::ShaderProgramId pro, const CoreGraphics::QueueType queue, const bool bindSharedResources);
}
namespace Vulkan
{
class VkShaderCache : public Resources::ResourceStreamCache
{
    __DeclareClass(VkShaderCache);
public:
    typedef Util::Dictionary<CoreGraphics::ShaderFeature::Mask, CoreGraphics::ShaderProgramId> ProgramMap;
    struct VkShaderRuntimeInfo
    {
        CoreGraphics::ShaderFeature::Mask activeMask;
        CoreGraphics::ShaderProgramId activeShaderProgram;
        ProgramMap programMap;
    };
    VkShaderCache();
    virtual ~VkShaderCache();
    CoreGraphics::ShaderProgramId GetShaderProgram(const CoreGraphics::ShaderId shaderId, const CoreGraphics::ShaderFeature::Mask mask);
    CoreGraphics::ResourceTableId CreateResourceTable(const CoreGraphics::ShaderId id, const IndexT group, const uint overallocationSize);
    CoreGraphics::BufferId CreateConstantBuffer(const CoreGraphics::ShaderId id, const Util::StringAtom& name, CoreGraphics::BufferAccessMode mode);
    CoreGraphics::BufferId CreateConstantBuffer(const CoreGraphics::ShaderId id, const IndexT cbIndex, CoreGraphics::BufferAccessMode mode);
    const IndexT GetConstantBinding(const CoreGraphics::ShaderId id, const Util::StringAtom& name) const;
    const IndexT GetConstantBinding(const CoreGraphics::ShaderId id, const IndexT cIndex) const;
    const SizeT GetConstantBindingsCount(const CoreGraphics::ShaderId id) const;
    CoreGraphics::ResourceTableLayoutId GetResourceTableLayout(const CoreGraphics::ShaderId id, const IndexT group);
    CoreGraphics::ResourcePipelineId GetResourcePipeline(const CoreGraphics::ShaderId id);
    const SizeT GetConstantCount(const CoreGraphics::ShaderId id) const;
    const CoreGraphics::ShaderConstantType GetConstantType(const CoreGraphics::ShaderId id, const IndexT i) const;
    const CoreGraphics::ShaderConstantType GetConstantType(const CoreGraphics::ShaderId id, const Util::StringAtom& name) const;
    const Util::StringAtom GetConstantBlockName(const CoreGraphics::ShaderId id, const Util::StringAtom& name);
    const Util::StringAtom GetConstantBlockName(const CoreGraphics::ShaderId id, const IndexT cIndex);
    const Util::StringAtom GetConstantName(const CoreGraphics::ShaderId id, const IndexT i) const;
    const IndexT GetConstantGroup(const CoreGraphics::ShaderId id, const Util::StringAtom& name) const;
    const IndexT GetConstantSlot(const CoreGraphics::ShaderId id, const Util::StringAtom& name) const;
    const SizeT GetConstantBufferCount(const CoreGraphics::ShaderId id) const;
    const SizeT GetConstantBufferSize(const CoreGraphics::ShaderId id, const IndexT i) const;
    const Util::StringAtom GetConstantBufferName(const CoreGraphics::ShaderId id, const IndexT i) const;
    const IndexT GetConstantBufferResourceSlot(const CoreGraphics::ShaderId id, const IndexT i) const;
    const IndexT GetConstantBufferResourceGroup(const CoreGraphics::ShaderId id, const IndexT i) const;
    const IndexT GetResourceSlot(const CoreGraphics::ShaderId id, const Util::StringAtom& name) const;
    const Util::Dictionary<CoreGraphics::ShaderFeature::Mask, CoreGraphics::ShaderProgramId>& GetPrograms(const CoreGraphics::ShaderId id);
    Util::String GetProgramName(CoreGraphics::ShaderProgramId id);
private:
    friend class VkVertexSignatureCache;
    friend class VkPipelineDatabase;
    friend void ::CoreGraphics::SetShaderProgram(const CoreGraphics::ShaderProgramId pro, const CoreGraphics::QueueType queue, const bool bindSharedResources);
    const VkProgramReflectionInfo& GetProgram(const CoreGraphics::ShaderProgramId shaderProgramId);
    LoadStatus LoadFromStream(const Resources::ResourceId id, const Util::StringAtom& tag, const Ptr<IO::Stream>& stream, bool immediate = false) override;
    LoadStatus ReloadFromStream(const Resources::ResourceId id, const Ptr<IO::Stream>& stream) override;
    void Unload(const Resources::ResourceId id) override;
    typedef Util::Dictionary<Util::StringAtom, CoreGraphics::BufferId> UniformBufferMap;
    typedef Util::Dictionary<uint32_t, Util::Array<CoreGraphics::BufferId>> UniformBufferGroupMap;
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
    struct VkShaderSetupInfo
    {
        VkDevice dev;
        Resources::ResourceName name;
        CoreGraphics::ShaderIdentifier::Code id;
        UniformBufferMap uniformBufferMap;
        UniformBufferGroupMap uniformBufferGroupMap;
        CoreGraphics::ResourcePipelineId pipelineLayout;
        Util::FixedArray<CoreGraphics::ResourcePipelinePushConstantRange> constantRangeLayout;
        Util::Array<CoreGraphics::SamplerId> immutableSamplers;
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
    };
    enum
    {
        Shader_ReflectionInfo,
        Shader_SetupInfo,
        Shader_RuntimeInfo,
        Shader_ProgramAllocator
    };
    Ids::IdAllocator<
        VkReflectionInfo,
        VkShaderSetupInfo,
        VkShaderRuntimeInfo,
        VkShaderProgramAllocator
    > shaderAlloc;
    __ImplementResourceAllocatorTyped(shaderAlloc, CoreGraphics::ShaderIdType);
    CoreGraphics::ShaderProgramId activeShaderProgram;
    CoreGraphics::ShaderFeature::Mask activeMask;
    Util::Dictionary<Ids::Id24, Ids::Id32> slicedStateMap;
};
}