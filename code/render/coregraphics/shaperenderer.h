#pragma once
#if __VULKAN__
#include "coregraphics/vk/vkshaperenderer.h"
namespace CoreGraphics
{
class ShapeRenderer : public Vulkan::VkShapeRenderer
{
    __DeclareClass(ShapeRenderer);
    __DeclareSingleton(ShapeRenderer);
public:
    ShapeRenderer();
    virtual ~ShapeRenderer();
};
}
#else
#error "Класс ShapeRenderer не реализован на этой платформе!"
#endif