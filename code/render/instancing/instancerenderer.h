#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#if __VULKAN__
#include "instancing/vk/vkinstancerenderer.h"
namespace Instancing
{
class InstanceRenderer : public Vulkan::VkInstanceRenderer
{
    __DeclareClass(InstanceRenderer);
    __DeclareSingleton(InstanceRenderer);
public:
    InstanceRenderer();
    virtual ~InstanceRenderer();
};
}
#else
#error "InstanceRenderer не реализован на этой платформе!"
#endif