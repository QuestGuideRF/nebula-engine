#pragma once
#include "ids/idallocator.h"
#include "util/hashtable.h"
namespace Vulkan
{
struct VertexLayoutVkBindInfo
{
    Util::FixedArray<VkVertexInputBindingDescription2EXT> binds;
    Util::FixedArray<VkVertexInputAttributeDescription2EXT> attrs;
};
const VertexLayoutVkBindInfo& VertexLayoutGetVkBindInfo(const CoreGraphics::VertexLayoutId layout);
struct DerivativeLayout
{
    VkPipelineVertexInputStateCreateInfo info;
    Util::Array<VkVertexInputAttributeDescription> attrs;
    DerivativeLayout()
    {
    }
    DerivativeLayout(const DerivativeLayout& rhs)
    {
        this->info = rhs.info;
        this->attrs = rhs.attrs;
        this->info.vertexAttributeDescriptionCount = this->attrs.Size();
        this->info.pVertexAttributeDescriptions = this->attrs.Begin();
    }
    DerivativeLayout(DerivativeLayout&& rhs)
    {
        this->info = rhs.info;
        this->attrs = std::move(rhs.attrs);
        this->info.vertexAttributeDescriptionCount = this->attrs.Size();
        this->info.pVertexAttributeDescriptions = this->attrs.Begin();
    }
    void operator=(const DerivativeLayout& rhs)
    {
        this->info = rhs.info;
        this->attrs = rhs.attrs;
        this->info.vertexAttributeDescriptionCount = this->attrs.Size();
        this->info.pVertexAttributeDescriptions = this->attrs.Begin();
    }
    void operator=(DerivativeLayout&& rhs)
    {
        this->info = rhs.info;
        this->attrs = std::move(rhs.attrs);
        this->info.vertexAttributeDescriptionCount = this->attrs.Size();
        this->info.pVertexAttributeDescriptions = this->attrs.Begin();
    }
};
struct BindInfo
{
    Util::FixedArray<VkVertexInputBindingDescription> binds;
    Util::FixedArray<VkVertexInputAttributeDescription> attrs;
};
enum
{
    VertexSignature_ProgramLayoutMapping
    , VertexSignature_VkPipelineInfo
    , VertexSignature_BindInfo
    , VertexSignature_LayoutInfo
    , VertexSignature_StreamSize
    , VertexSignature_DynamicBindInfo
};
typedef Ids::IdAllocator<
    Util::HashTable<uint64_t, DerivativeLayout>,
    VkPipelineVertexInputStateCreateInfo,
    BindInfo,
    CoreGraphics::VertexLayoutInfo,
    Util::Array<SizeT>,
    VertexLayoutVkBindInfo
> VkVertexLayoutAllocator;
extern VkVertexLayoutAllocator vertexLayoutAllocator;
}