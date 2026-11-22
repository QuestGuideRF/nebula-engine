#pragma once
#include "coregraphics/shader.h"
#include "coregraphics/pass.h"
#include "coregraphics/buffer.h"
#include "coregraphics/resourcetable.h"
namespace Vulkan
{
struct VkPassLoadInfo
{
    VkDevice dev;
    Util::StringAtom name;
    Util::Array<CoreGraphics::TextureViewId> attachments;
    Util::Array<Math::vec4> attachmentClears;
    Util::Array<CoreGraphics::AttachmentFlagBits> attachmentFlags;
    Util::Array<bool> attachmentIsDepthStencil;
    Util::Array<CoreGraphics::Subpass> subpasses;
    VkRect2D renderArea;
    VkFramebuffer framebuffer;
    VkRenderPass pass;
    Util::FixedArray<VkRect2D> rects;
    Util::FixedArray<VkViewport> viewports;
    Util::FixedArray<VkClearValue> clearValues;
};
struct VkPassRuntimeInfo
{
    CoreGraphics::BufferId passBlockBuffer;
    IndexT renderTargetDimensionsVar;
    VkGraphicsPipelineCreateInfo framebufferPipelineInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineViewportStateCreateInfo viewportInfo;
    uint32_t currentSubpassIndex;
    CoreGraphics::ResourceTableId passDescriptorSet;
    CoreGraphics::ResourcePipelineId passPipelineLayout;
    Util::FixedArray<VkPipelineViewportStateCreateInfo> subpassPipelineInfo;
    CoreGraphics::PassRecordMode recordMode;
};
enum
{
    Pass_VkLoadInfo
    , Pass_VkRuntimeInfo
    , Pass_VkRenderPassBeginInfo
    , Pass_SubpassAttachments
};
typedef Ids::IdAllocator<
    VkPassLoadInfo,
    VkPassRuntimeInfo,
    VkRenderPassBeginInfo,
    Util::Array<uint32_t>
> VkPassAllocator;
extern VkPassAllocator passAllocator;
const VkRenderPassBeginInfo& PassGetVkRenderPassBeginInfo(const CoreGraphics::PassId id);
const VkGraphicsPipelineCreateInfo& PassGetVkFramebufferInfo(const CoreGraphics::PassId id);
const VkPipelineViewportStateCreateInfo& PassGetVkViewportInfo(const CoreGraphics::PassId id, uint32_t subpass);
const SizeT PassGetVkNumAttachments(const CoreGraphics::PassId id);
const VkDevice PassGetVkDevice(const CoreGraphics::PassId id);
const VkFramebuffer PassGetVkFramebuffer(const CoreGraphics::PassId id);
const VkRenderPass PassGetVkRenderPass(const CoreGraphics::PassId id);
}