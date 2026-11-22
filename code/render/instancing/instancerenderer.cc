//------------------------------------------------------------------------------
//  instancerenderer.cc
//  (C) 2012 Густав Стербрант
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "instancerenderer.h"
namespace Instancing
{
#if __VULKAN__
__ImplementClass(Instancing::InstanceRenderer, 'INRD', Vulkan::VkInstanceRenderer);
#else
#error "Класс InstanceRenderer не реализован на этой платформе!"
#endif

__ImplementSingleton(Instancing::InstanceRenderer);
//------------------------------------------------------------------------------
/**
*/
InstanceRenderer::InstanceRenderer()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
InstanceRenderer::~InstanceRenderer()
{
    __DestructSingleton;
}

} // пространство имен
