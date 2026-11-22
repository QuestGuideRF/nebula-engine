//------------------------------------------------------------------------------
//  displaydevice.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/displaydevice.h"

namespace CoreGraphics
{
#if __VULKAN__
__ImplementClass(CoreGraphics::DisplayDevice, 'DDVC', GLFW::GLFWDisplayDevice);
__ImplementSingleton(CoreGraphics::DisplayDevice);
#else
#error "Класс DisplayDevice не реализован на этой платформе!"
#endif

//------------------------------------------------------------------------------
/**
*/
DisplayDevice::DisplayDevice()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
DisplayDevice::~DisplayDevice()
{
    __DestructSingleton;
}

} // пространство имен CoreGraphics

