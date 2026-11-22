#pragma once
#if __VULKAN__
#include "coregraphics/vk/vkshaderserver.h"
namespace CoreGraphics
{
class ShaderServer : public Vulkan::VkShaderServer
{
    __DeclareClass(ShaderServer);
    __DeclareSingleton(ShaderServer);
public:
    ShaderServer();
    virtual ~ShaderServer();
};
}
#else
#error "Класс ShaderServer не реализован на этой платформе!"
#endif