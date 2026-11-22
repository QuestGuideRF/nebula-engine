//------------------------------------------------------------------------------
//  textrenderer.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/textrenderer.h"

namespace CoreGraphics
{
#if __VULKAN__
__ImplementClass(CoreGraphics::TextRenderer, 'TXRR', Vulkan::VkTextRenderer);
#else
#error "Класс TextRenderer не реализован на этой платформе!"
#endif

__ImplementSingleton(CoreGraphics::TextRenderer);

//------------------------------------------------------------------------------
/**
*/
TextRenderer::TextRenderer()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
TextRenderer::~TextRenderer()
{
    __DestructSingleton;
}

} // пространство имен CoreGraphics
