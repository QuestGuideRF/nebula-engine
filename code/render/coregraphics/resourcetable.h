#pragma once
#include "ids/id.h"
#include "ids/idpool.h"
#include "texture.h"
#include "textureview.h"
#include "buffer.h"
#include "sampler.h"
#include "accelerationstructure.h"
#include "coregraphics/config.h"
namespace CoreGraphics
{
ID_24_8_TYPE(ResourceTableLayoutId);
struct ResourceTableLayoutTexture
{
    IndexT slot;
    SizeT num;
    CoreGraphics::ShaderVisibility visibility;
    CoreGraphics::SamplerId immutableSampler;
};
struct ResourceTableLayoutConstantBuffer
{
    IndexT slot;
    SizeT num;
    CoreGraphics::ShaderVisibility visibility;
    bool dynamicOffset;
};
struct ResourceTableLayoutShaderRWBuffer
{
    IndexT slot;
    SizeT num;
    CoreGraphics::ShaderVisibility visibility;
    bool dynamicOffset;
};
struct ResourceTableLayoutAccelerationStructure
{
    IndexT slot;
    SizeT num;
    CoreGraphics::ShaderVisibility visibility;
};
struct ResourceTableLayoutSampler
{
    IndexT slot;
    CoreGraphics::ShaderVisibility visibility;
    CoreGraphics::SamplerId sampler;
};
struct ResourceTableLayoutInputAttachment
{
    IndexT slot;
    SizeT num;
    CoreGraphics::ShaderVisibility visibility;
};
struct ResourceTableLayoutCreateInfo
{
    Util::Array<ResourceTableLayoutTexture> textures;
    Util::Array<ResourceTableLayoutTexture> rwTextures;
    Util::Array<ResourceTableLayoutConstantBuffer> constantBuffers;
    Util::Array<ResourceTableLayoutShaderRWBuffer> rwBuffers;
    Util::Array<ResourceTableLayoutAccelerationStructure> accelerationStructures;
    Util::Array<ResourceTableLayoutSampler> samplers;
    Util::Array<ResourceTableLayoutInputAttachment> inputAttachments;
    uint32_t descriptorPoolInitialGrow = 1;
};
ResourceTableLayoutId CreateResourceTableLayout(const ResourceTableLayoutCreateInfo& info);
void DestroyResourceTableLayout(const ResourceTableLayoutId& id);
ID_24_8_TYPE(ResourceTableId);
extern Util::Array<CoreGraphics::ResourceTableId> PendingTableCommits;
extern bool ResourceTableBlocked;
extern Threading::CriticalSection PendingTableCommitsLock;
struct ResourceTableTexture
{
    ResourceTableTexture()
        : tex(InvalidTextureId)
        , slot(0)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
        , isStencil(false)
    {};
    ResourceTableTexture(const CoreGraphics::TextureId tex, IndexT slot)
        : tex(tex)
        , slot(slot)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
        , isStencil(false)
    {};
    ResourceTableTexture(const CoreGraphics::TextureId tex, IndexT slot, IndexT index)
        : tex(tex)
        , slot(slot)
        , index(index)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
        , isStencil(false)
    {};
    ResourceTableTexture(const CoreGraphics::TextureId tex, IndexT slot, bool isDepth, bool isStencil)
        : tex(tex)
        , slot(slot)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(isDepth)
        , isStencil(isStencil)
    {};
    ResourceTableTexture(const CoreGraphics::TextureId tex, IndexT slot, IndexT index, CoreGraphics::SamplerId sampler, bool isDepth = false, bool isStencil = false)
        : tex(tex)
        , slot(slot)
        , index(index)
        , sampler(sampler)
        , isDepth(isDepth)
        , isStencil(isStencil)
    {};
    CoreGraphics::TextureId tex;
    IndexT slot;
    IndexT index;
    CoreGraphics::SamplerId sampler;
    bool isDepth : 1;
    bool isStencil : 1;
};
struct ResourceTableTextureView
{
    ResourceTableTextureView()
        : tex(InvalidTextureViewId)
        , slot(0)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
        , isStencil(false)
    {};
    ResourceTableTextureView(const CoreGraphics::TextureViewId tex, IndexT slot)
        : tex(tex)
        , slot(slot)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
        , isStencil(false)
    {};
    ResourceTableTextureView(const CoreGraphics::TextureViewId tex, IndexT slot, IndexT index)
        : tex(tex)
        , slot(slot)
        , index(index)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
        , isStencil(false)
    {};
    ResourceTableTextureView(const CoreGraphics::TextureViewId tex, IndexT slot, bool isDepth, bool isStencil)
        : tex(tex)
        , slot(slot)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(isDepth)
        , isStencil(isStencil)
    {};
    ResourceTableTextureView(const CoreGraphics::TextureViewId tex, IndexT slot, IndexT index, CoreGraphics::SamplerId sampler, bool isDepth = false, bool isStencil = false)
        : tex(tex)
        , slot(slot)
        , index(index)
        , sampler(sampler)
        , isDepth(isDepth)
        , isStencil(isStencil)
    {};
    CoreGraphics::TextureViewId tex;
    IndexT slot;
    IndexT index;
    CoreGraphics::SamplerId sampler;
    bool isDepth : 1;
    bool isStencil : 1;
};
struct ResourceTableInputAttachment
{
    ResourceTableInputAttachment()
        : tex(InvalidTextureViewId)
        , slot(0)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
    {};
    ResourceTableInputAttachment(const CoreGraphics::TextureViewId tex, IndexT slot)
        : tex(tex)
        , slot(slot)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(false)
    {};
    ResourceTableInputAttachment(const CoreGraphics::TextureViewId tex, IndexT slot, bool isDepth = false)
        : tex(tex)
        , slot(slot)
        , index(0)
        , sampler(CoreGraphics::InvalidSamplerId)
        , isDepth(isDepth)
    {};
    ResourceTableInputAttachment(const CoreGraphics::TextureViewId tex, IndexT slot, IndexT index, CoreGraphics::SamplerId sampler, bool isDepth = false)
        : tex(tex)
        , slot(slot)
        , index(index)
        , sampler(sampler)
        , isDepth(isDepth)
    {};
    CoreGraphics::TextureViewId tex;
    IndexT slot;
    IndexT index;
    CoreGraphics::SamplerId sampler;
    bool isDepth : 1;
};
struct ResourceTableBuffer
{
    ResourceTableBuffer()
        : buf(InvalidBufferId)
        , slot(0)
        , index(0)
        , size(NEBULA_WHOLE_BUFFER_SIZE)
        , offset(0)
        , texelBuffer(false)
        , dynamicOffset(false)
    {};
    ResourceTableBuffer(const CoreGraphics::BufferId buf, IndexT slot)
        : buf(buf)
        , slot(slot)
        , index(0)
        , size(NEBULA_WHOLE_BUFFER_SIZE)
        , offset(0)
        , texelBuffer(false)
        , dynamicOffset(false)
    {};
    ResourceTableBuffer(const CoreGraphics::BufferId buf, IndexT slot, uint64_t size)
        : buf(buf)
        , slot(slot)
        , index(0)
        , size(size)
        , offset(0)
        , texelBuffer(false)
        , dynamicOffset(false)
    {};
    ResourceTableBuffer(const CoreGraphics::BufferId buf, IndexT slot, uint64_t size, uint64_t offset)
        : buf(buf)
        , slot(slot)
        , index(0)
        , size(size)
        , offset(offset)
        , texelBuffer(false)
        , dynamicOffset(false)
    {};
    ResourceTableBuffer(const CoreGraphics::BufferId buf, IndexT slot, SizeT index, uint64_t size, uint64_t offset, bool texelBuffer = false, bool dynamicOffset = false)
        : buf(buf)
        , slot(slot)
        , index(index)
        , size(size)
        , offset(offset)
        , texelBuffer(texelBuffer)
        , dynamicOffset(dynamicOffset)
    {};
    CoreGraphics::BufferId buf;
    IndexT slot;
    IndexT index;
    uint64_t size;
    uint64_t offset;
    bool texelBuffer;
    bool dynamicOffset;
};
struct ResourceTableSampler
{
    CoreGraphics::SamplerId samp;
    IndexT slot;
};
struct ResourceTableTlas
{
    ResourceTableTlas(const CoreGraphics::TlasId tlas, IndexT slot)
        : tlas(tlas)
        , slot(slot)
    {};
    CoreGraphics::TlasId tlas;
    IndexT slot;
};
struct ResourceTableCreateInfo
{
    const char* name = nullptr;
    ResourceTableLayoutId layout;
    uint overallocationSize = 256;
};
ResourceTableId CreateResourceTable(const ResourceTableCreateInfo& info);
void DestroyResourceTable(const ResourceTableId id);
const ResourceTableLayoutId& ResourceTableGetLayout(CoreGraphics::ResourceTableId id);
void ResourceTableSetTexture(const ResourceTableId id, const ResourceTableTexture& tex);
void ResourceTableSetTexture(const ResourceTableId id, const ResourceTableTextureView& tex);
void ResourceTableSetInputAttachment(const ResourceTableId id, const ResourceTableInputAttachment& tex);
void ResourceTableSetRWTexture(const ResourceTableId id, const ResourceTableTexture& tex);
void ResourceTableSetRWTexture(const ResourceTableId id, const ResourceTableTextureView& tex);
void ResourceTableSetConstantBuffer(const ResourceTableId id, const ResourceTableBuffer& buf);
void ResourceTableSetRWBuffer(const ResourceTableId id, const ResourceTableBuffer& buf);
void ResourceTableSetSampler(const ResourceTableId id, const ResourceTableSampler& samp);
void ResourceTableSetAccelerationStructure(const ResourceTableId id, const ResourceTableTlas& tlas);
void ResourceTableCopy(const ResourceTableId from, const IndexT fromSlot, const IndexT fromIndex, const ResourceTableId to, const IndexT toSlot, const IndexT toIndex, const SizeT numResources);
void ResourceTableBlock(bool b);
void ResourceTableCommitChanges(const ResourceTableId id);
struct ResourceTableSet
{
    ResourceTableSet() {};
    ResourceTableSet(const ResourceTableCreateInfo& createInfo);
    ResourceTableSet(ResourceTableSet&& rhs);
    ~ResourceTableSet();
    void Discard();
    void ForEach(std::function<void(const ResourceTableId, const IndexT)> func);
    void operator=(ResourceTableSet&& rhs);
    const CoreGraphics::ResourceTableId Get();
    Util::FixedArray<CoreGraphics::ResourceTableId> tables;
};
ID_24_8_TYPE(ResourcePipelineId);
struct ResourcePipelinePushConstantRange
{
    SizeT size;
    SizeT offset;
    CoreGraphics::ShaderVisibility vis;
};
struct ResourcePipelineCreateInfo
{
    Util::Array<ResourceTableLayoutId> tables;
    Util::Array<uint32_t> indices;
    ResourcePipelinePushConstantRange push;
};
ResourcePipelineId CreateResourcePipeline(const ResourcePipelineCreateInfo& info);
void DestroyResourcePipeline(const ResourcePipelineId& id);
}