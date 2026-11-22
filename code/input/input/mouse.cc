//------------------------------------------------------------------------------
//  mouse.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/mouse.h"
#if __VULKAN__
namespace Input
{
__ImplementClass(Input::Mouse, 'MOUS', Base::MouseBase);
}
#elif __WIN32__
namespace Input
{
__ImplementClass(Input::Mouse, 'MOUS', Win32::Win32Mouse);
}
#else
#error "Класс мыши не реализован на этой платформе!"
#endif
