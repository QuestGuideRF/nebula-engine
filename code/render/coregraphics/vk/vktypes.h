#pragma once
#include "core/types.h"
#include "coregraphics/pixelformat.h"
#include "coregraphics/vertexcomponent.h"
#include "coregraphics/primitivetopology.h"
#include "coregraphics/config.h"
namespace Vulkan
{
static const VkComponentSwizzle VkSwizzle[] =
{
    VK_COMPONENT_SWIZZLE_IDENTITY,
    VK_COMPONENT_SWIZZLE_R,
    VK_COMPONENT_SWIZZLE_G,
    VK_COMPONENT_SWIZZLE_B,
    VK_COMPONENT_SWIZZLE_A,
    VK_COMPONENT_SWIZZLE_ZERO,
    VK_COMPONENT_SWIZZLE_ONE,
};
class VkTypes
{
public:
    struct VkBlockDimensions
    {
        uint32_t width;
        uint32_t height;
    };
#pragma region Pixel stuff
    static VkFormat AsVkFormat(CoreGraphics::PixelFormat::Code p);
    static bool IsCompressedFormat(VkFormat p);
    static VkBlockDimensions AsVkBlockSize(CoreGraphics::PixelFormat::Code p);
    static VkBlockDimensions AsVkBlockSize(VkFormat fmt);
    static VkFormat AsVkMappableImageFormat(VkFormat fmt);
    static VkFormat AsVkDataFormat(CoreGraphics::PixelFormat::Code p);
    static VkSampleCountFlagBits AsVkSampleFlags(const SizeT samples);
    static VkImageType AsVkImageType(CoreGraphics::TextureType type);
    static VkImageViewType AsVkImageViewType(CoreGraphics::TextureType type);
    static CoreGraphics::PixelFormat::Code AsNebulaPixelFormat(VkFormat f);
    static VkImageAspectFlags AsVkImageAspectFlags(const CoreGraphics::ImageBits bits);
    static VkShaderStageFlags AsVkShaderVisibility(const CoreGraphics::ShaderVisibility vis);
    static VkImageLayout AsVkImageLayout(const CoreGraphics::ImageLayout layout);
    static VkPipelineStageFlags AsVkPipelineStage(const CoreGraphics::PipelineStage stage);
    static VkAccessFlags AsVkAccessFlags(const CoreGraphics::PipelineStage stage);
    static VkImageLayout AsVkImageLayout(const CoreGraphics::PipelineStage stage, bool depthStencil = false);
#pragma endregion
    static VkPrimitiveTopology AsVkPrimitiveType(CoreGraphics::PrimitiveTopology::Code t);
    static uint32_t AsVkSize(CoreGraphics::VertexComponent::Format f);
    static uint32_t AsVkNumComponents(CoreGraphics::VertexComponent::Format f);
    static VkFormat AsVkVertexType(CoreGraphics::VertexComponent::Format f);
};
}