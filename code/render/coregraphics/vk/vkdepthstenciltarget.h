#pragma once
#include "core/refcounted.h"
#include "coregraphics/base/depthstenciltargetbase.h"
namespace Vulkan
{
class VkDepthStencilTarget : public Base::DepthStencilTargetBase
{
    __DeclareClass(VkDepthStencilTarget);
public:
    VkDepthStencilTarget();
    virtual ~VkDepthStencilTarget();
    void Setup();
    void Discard();
    void OnDisplayResized(SizeT width, SizeT height);
    void BeginPass();
    void EndPass();
    const VkViewport& GetVkViewport() const;
    const VkRect2D& GetVkScissorRect() const;
    VkImageView GetVkImageView();
private:
    VkViewport viewport;
    VkRect2D scissor;
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};
inline const VkViewport&
VkDepthStencilTarget::GetVkViewport() const
{
    return this->viewport;
}
inline const VkRect2D&
VkDepthStencilTarget::GetVkScissorRect() const
{
    return this->scissor;
}
inline VkImageView
VkDepthStencilTarget::GetVkImageView()
{
    return this->view;
}
}