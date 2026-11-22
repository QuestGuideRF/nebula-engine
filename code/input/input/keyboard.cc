//------------------------------------------------------------------------------
//  keyboard.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "input/keyboard.h"
#if __WIN32__ || __LINUX__
namespace Input
{
__ImplementClass(Input::Keyboard, 'KBRD', Base::KeyboardBase);
}
#else
#error "Класс клавиатуры не реализован на этой платформе!"
#endif
