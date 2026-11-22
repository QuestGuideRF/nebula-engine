#pragma once
#if __VULKAN__
#include "coregraphics/vk/vktextrenderer.h"
namespace CoreGraphics
{
class TextRenderer : public Vulkan::VkTextRenderer
{
    __DeclareClass(TextRenderer);
    __DeclareSingleton(TextRenderer);
public:
    TextRenderer();
    virtual ~TextRenderer();
};
}
#else
#error "Класс TextRenderer не реализован на этой платформе!"
#endif