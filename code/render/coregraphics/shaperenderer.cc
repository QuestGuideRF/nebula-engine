//------------------------------------------------------------------------------
//  shaperenderer.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/shaperenderer.h"

namespace CoreGraphics
{
#if __VULKAN__
__ImplementClass(CoreGraphics::ShapeRenderer, 'SHPR', Vulkan::VkShapeRenderer);
#else
#error "Класс ShapeRenderer не реализован на этой платформе!"
#endif
__ImplementSingleton(CoreGraphics::ShapeRenderer);

//------------------------------------------------------------------------------
/**
*/
ShapeRenderer::ShapeRenderer()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
ShapeRenderer::~ShapeRenderer()
{
    __DestructSingleton;
}

} // пространство имен CoreGraphics
    
