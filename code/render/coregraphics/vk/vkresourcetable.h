#pragma once
#include "ids/idallocator.h"
#include "coregraphics/resourcetable.h"
#include "threading/spinlock.h"
#include <array>
namespace Vulkan
{
enum class WriteType
{
    Image,
    Buffer,
    TexelBuffer,
    Tlas
};
struct WriteInfo
{
    union
    {
        VkDescriptorImageInfo img;
        VkDescriptorBufferInfo buf;
        VkBufferView tex;
        VkAccelerationStructureKHR tlas;
    };
    VkWriteDescriptorSet write;
    VkWriteDescriptorSetAccelerationStructureKHR tlasWrite;
    WriteType type;
};
enum
{
    ResourceTable_Device,
    ResourceTable_DescriptorSet,
    ResourceTable_DescriptorPoolIndex,
    ResourceTable_Lock,
    ResourceTable_Layout,
    ResourceTable_WriteInfos,
    ResourceTable_Copies
};
typedef Ids::IdAllocator<
    VkDevice,
    VkDescriptorSet,
    IndexT,
    Threading::Spinlock,
    CoreGraphics::ResourceTableLayoutId,
    Util::Array<WriteInfo, 16>,
    Util::Array<VkCopyDescriptorSet, 4>
> VkResourceTableAllocator;
extern VkResourceTableAllocator resourceTableAllocator;
const VkDescriptorSet& ResourceTableGetVkDescriptorSet(CoreGraphics::ResourceTableId id);
const IndexT ResourceTableGetVkPoolIndex(CoreGraphics::ResourceTableId id);
const VkDevice& ResourceTableGetVkDevice(CoreGraphics::ResourceTableId id);
enum
{
    ResourceTableLayout_Device,
    ResourceTableLayout_SetLayout,
    ResourceTableLayout_PoolSizes,
    ResourceTableLayout_Samplers,
    ResourceTableLayout_ImmutableSamplerFlags,
    ResourceTableLayout_DescriptorPools,
    ResourceTableLayout_DescriptorPoolFreeItems,
};
typedef Ids::IdAllocator<
    VkDevice,
    VkDescriptorSetLayout,
    Util::Array<VkDescriptorPoolSize>,
    Util::Array<Util::Pair<CoreGraphics::SamplerId, uint32_t>>,
    Util::HashTable<uint32_t, bool>,
    Util::Array<VkDescriptorPool>,
    Util::Array<uint>
> VkResourceTableLayoutAllocator;
extern VkResourceTableLayoutAllocator resourceTableLayoutAllocator;
extern VkDescriptorSetLayout EmptySetLayout;
void SetupEmptyDescriptorSetLayout();
const VkDescriptorSetLayout& ResourceTableLayoutGetVk(const CoreGraphics::ResourceTableLayoutId& id);
void ResourceTableLayoutAllocTable(const CoreGraphics::ResourceTableLayoutId& id, const VkDevice dev, uint overallocationSize, IndexT& outIndex, VkDescriptorSet& outSet);
void ResourceTableLayoutDeallocTable(const CoreGraphics::ResourceTableLayoutId& id, const VkDevice dev, const VkDescriptorSet& set, const IndexT index);
const VkDescriptorPool& ResourceTableLayoutGetVkDescriptorPool(const CoreGraphics::ResourceTableLayoutId& id, const IndexT index);
uint* ResourceTableLayoutGetFreeItemsCounter(const CoreGraphics::ResourceTableLayoutId& id, const IndexT index);
typedef Ids::IdAllocator<
    VkDevice,
    VkPipelineLayout
> VkResourcePipelineAllocator;
extern VkResourcePipelineAllocator resourcePipelineAllocator;
const VkPipelineLayout& ResourcePipelineGetVk(const CoreGraphics::ResourcePipelineId& id);
}