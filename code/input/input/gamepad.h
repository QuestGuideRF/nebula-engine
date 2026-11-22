#pragma once
#if __VULKAN__
#include "input/base/gamepadbase.h"
namespace Input
{
    class GamePad : public Base::GamePadBase
    {
        __DeclareClass(GamePad);
    };
}
#elif __WIN32__
#include "input/xinput/xinputgamepad.h"
namespace Input
{
class GamePad : public XInput::XInputGamePad
{
    __DeclareClass(GamePad);
};
}
#else
#error "Класс GamePad не реализован на этой платформе!"
#endif