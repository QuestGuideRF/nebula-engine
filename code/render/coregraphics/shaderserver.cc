//------------------------------------------------------------------------------
//  shaderserver.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/shaderserver.h"

namespace CoreGraphics
{
#if __VULKAN__
__ImplementClass(CoreGraphics::ShaderServer, 'SHSV', Vulkan::VkShaderServer);
__ImplementSingleton(CoreGraphics::ShaderServer);
#else
#error "Класс ShaderServer не реализован на этой платформе!"
#endif

//------------------------------------------------------------------------------
/**
*/
ShaderServer::ShaderServer()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
ShaderServer::~ShaderServer()
{
    __DestructSingleton;
}

} // пространство имен CoreGraphics
