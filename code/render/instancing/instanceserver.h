#pragma once
#if __VULKAN__
#include "instancing/vk/vkinstanceserver.h"
namespace Instancing
{
class InstanceServer : public Vulkan::VkInstanceServer
{
    __DeclareClass(InstanceServer);
    __DeclareSingleton(InstanceServer);
public:
    InstanceServer();
    virtual ~InstanceServer();
};
}
#else
#error "InstanceServer не реализован на этой платформе!"
#endif