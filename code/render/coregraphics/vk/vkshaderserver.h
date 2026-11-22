#pragma once
#include "core/refcounted.h"
#include "util/fixedpool.h"
#include "coregraphics/base/shaderserverbase.h"
#include "coregraphics/config.h"
#include "coregraphics/texture.h"
#include "effectfactory.h"
#include "system_shaders/shared.h"
namespace Graphics
{
class FrameScript;
}
namespace Vulkan
{
class VkShaderServer : public Base::ShaderServerBase
{
    __DeclareClass(VkShaderServer);
    __DeclareInterfaceSingleton(VkShaderServer);
public:
    VkShaderServer();
    virtual ~VkShaderServer();
    bool Open();
    void Close();
    void AddPendingImageView(CoreGraphics::TextureId tex, VkImageViewCreateInfo viewCreate, uint32_t bind);
    void UpdateResources();
private:
    Threading::CriticalSection bindResourceCriticalSection;
    struct _PendingView
    {
        CoreGraphics::TextureId tex;
        VkImageViewCreateInfo createInfo;
        uint32_t bind;
    };
    struct _PendingViewDelete
    {
        VkImageView view;
        uint32_t replaceCounter;
    };
    Threading::SafeQueue<_PendingView> pendingViews;
    Util::Array<_PendingViewDelete> pendingViewDeletes;
    AnyFX::EffectFactory* factory;
};
}