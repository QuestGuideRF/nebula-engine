//------------------------------------------------------------------------------
//  gamepad.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/gamepad.h"
#if __VULKAN__
namespace Input
{
__ImplementClass(Input::GamePad, 'GMPD', Base::GamePadBase);
}
#elif __WIN32__
namespace Input
{
__ImplementClass(Input::GamePad, 'GMPD', XInput::XInputGamePad);
}
#else
#error "Класс GamePad не реализован на этой платформе!"
#endif
