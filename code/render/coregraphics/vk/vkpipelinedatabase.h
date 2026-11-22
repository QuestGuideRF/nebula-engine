#pragma once
#include "core/singleton.h"
#include "coregraphics/shader.h"
#include "coregraphics/pass.h"
#include "memory/arenaallocator.h"
namespace Vulkan
{
class VkVertexLayout;
class VkShaderProgram;
class VkPass;
class VkPipelineDatabase
{
    __DeclareSingleton(VkPipelineDatabase);
public:
    enum StateLevel
    {
        PassLevel,
        SubpassLevel,
        ShaderLevel,
        VertexLayoutLevel,
        InputAssemblyLevel,
        PipelineLevel
    };
    VkPipelineDatabase();
    virtual ~VkPipelineDatabase();
    void Setup(const VkDevice dev, const VkPipelineCache cache);
    void Discard();
    void SetPass(const CoreGraphics::PassId pass);
    void SetSubpass(uint32_t subpass);
    void SetShader(const CoreGraphics::ShaderProgramId program, const VkGraphicsPipelineCreateInfo& shaderInfo);
    void SetInputAssembly(const CoreGraphics::InputAssemblyKey key);
    VkPipeline GetCompiledPipeline();
    VkPipeline GetCompiledPipeline(
        const CoreGraphics::PassId pass
        , const uint32_t subpass
        , const CoreGraphics::ShaderProgramId program
        , const CoreGraphics::InputAssemblyKey inputAssembly
        , const VkGraphicsPipelineCreateInfo& shaderInfo);
    VkPipeline CreatePipeline(
        const CoreGraphics::PassId pass
        , const uint32_t subpass
        , const CoreGraphics::ShaderProgramId program
        , const CoreGraphics::InputAssemblyKey inputAssembly
        , const VkGraphicsPipelineCreateInfo& shaderInfo
    );
    void Reset();
    void Reload(const CoreGraphics::ShaderProgramId id);
    void RecreatePipelines();
private:
    VkDevice dev;
    VkPipelineCache cache;
    CoreGraphics::PassId currentPass;
    uint32_t currentSubpass;
    CoreGraphics::ShaderProgramId currentShaderProgram;
    VkGraphicsPipelineCreateInfo currentShaderInfo;
    CoreGraphics::InputAssemblyKey currentInputAssembly;
    const VkPipelineVertexInputStateCreateInfo* currentVertexLayout;
    struct Tier1Node;
    struct Tier2Node;
    struct Tier3Node;
    struct Tier4Node;
    struct BaseNode
    {
        bool initial;
        BaseNode() : initial(true) {}
    };
    struct Tier1Node : public BaseNode
    {
        Util::Dictionary<IndexT, Tier2Node*> children;
    };
    struct Tier2Node : public BaseNode
    {
        Util::Dictionary<CoreGraphics::ShaderProgramId, Tier3Node*> children;
    };
    struct Tier3Node : public BaseNode
    {
        Util::Dictionary<CoreGraphics::InputAssemblyKey, Tier4Node*> children;
    };
    struct Tier4Node : public BaseNode
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
    };
    Util::Dictionary<CoreGraphics::PassId, Tier1Node*> tier1;
    Memory::ArenaAllocator<BIG_CHUNK> tierNodeAllocator;
    Tier1Node* ct1;
    Tier2Node* ct2;
    Tier3Node* ct3;
    Tier4Node* ct4;
    VkPipeline currentPipeline;
};
}