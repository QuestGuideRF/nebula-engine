#pragma once
#if __WIN32__ || __LINUX__
#include "input/base/keyboardbase.h"
namespace Input
{
class Keyboard : public Base::KeyboardBase
{
    __DeclareClass(Keyboard);
};
}
#else
#error "Класс клавиатуры не реализован на этой платформе!"
#endif