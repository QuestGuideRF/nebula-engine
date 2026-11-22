#pragma once
#include "coregraphics/graphicsdevice.h"
#include "vkbarrier.h"
#include "vksubcontexthandler.h"
#include "vkcommandbufferthread.h"
#include "vkmemory.h"
namespace Vulkan
{
void SetupAdapter(CoreGraphics::GraphicsDeviceCreateInfo::Features features);
VkInstance GetInstance();
VkDevice GetCurrentDevice();
VkPhysicalDevice GetCurrentPhysicalDevice();
VkPhysicalDeviceFeatures GetCurrentFeatures();
VkPipelineCache GetPipelineCache();
VkPhysicalDeviceMemoryProperties GetMemoryProperties();
VkCommandBuffer GetMainBuffer(const CoreGraphics::QueueType queue);
VkSemaphore GetRenderingSemaphore();
VkFence GetPresentFence();
void DelayedDeleteVkBuffer(const VkDevice dev, const VkBuffer buf);
VkQueryPool GetQueryPool(const CoreGraphics::QueryType query);
const VkQueue GetQueue(const CoreGraphics::QueueType type, const IndexT index);
const VkQueue GetCurrentQueue(const CoreGraphics::QueueType type);
VkPipeline GetOrCreatePipeline(CoreGraphics::PassId pass, uint subpass, CoreGraphics::ShaderProgramId program, const CoreGraphics::InputAssemblyKey inputAssembly, const VkGraphicsPipelineCreateInfo& info);
void SparseTextureBind(const VkImage img, const Util::Array<VkSparseMemoryBind>& opaqueBinds, const Util::Array<VkSparseImageMemoryBind>& pageBinds);
void SparseBufferBind(const VkBuffer buf, const Util::Array<VkSparseMemoryBind>& binds);
void ClearPending();
void DeviceLost();
}