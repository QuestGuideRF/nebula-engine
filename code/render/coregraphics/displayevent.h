#pragma once
#include "core/types.h"
#include "input/key.h"
#include "input/char.h"
#include "input/mousebutton.h"
#include "math/vec2.h"
#include "coregraphics/window.h"
namespace CoreGraphics
{
class DisplayEvent
{
public:
    enum Code
    {
        InvalidCode,
        WindowOpen,
        WindowClose,
        WindowReopen,
        CloseRequested,
        WindowMinimized,
        WindowRestored,
        ToggleFullscreenWindowed,
        WindowResized,
        SetCursor,
        Paint,
        SetFocus,
        KillFocus,
        KeyDown,
        KeyUp,
        Character,
        MouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseButtonDoubleClick,
        MouseWheelForward,
        MouseWheelBackward,
    };
    DisplayEvent();
    DisplayEvent(Code c);
    DisplayEvent(Code c, CoreGraphics::WindowId wnd);
    DisplayEvent(Code c, const Math::vec2& absPos, const Math::vec2& normPos);
    DisplayEvent(Code c, Input::Key::Code k);
    DisplayEvent(Code c, Input::Char chr);
    DisplayEvent(Code c, Input::MouseButton::Code b, const Math::vec2& absPos, const Math::vec2& normPos);
    Code GetEventCode() const;
    CoreGraphics::WindowId GetWindowId() const;
    const Math::vec2& GetAbsMousePos() const;
    const Math::vec2& GetNormMousePos() const;
    Input::Key::Code GetKey() const;
    Input::Char GetChar() const;
    Input::MouseButton::Code GetMouseButton() const;
private:
    Code code;
    CoreGraphics::WindowId windowId;
    Math::vec2 absMousePos;
    Math::vec2 normMousePos;
    Input::Key::Code keyCode;
    Input::Char charCode;
    Input::MouseButton::Code mouseButtonCode;
};
inline
DisplayEvent::DisplayEvent() :
    code(InvalidCode),
    windowId(Ids::InvalidId32),
    absMousePos(0.0f, 0.0f),
    normMousePos(0.0f, 0.0f),
    keyCode(Input::Key::InvalidKey),
    charCode(0),
    mouseButtonCode(Input::MouseButton::InvalidMouseButton)
{
}
inline
DisplayEvent::DisplayEvent(Code c) :
    code(c),
    windowId(Ids::InvalidId32),
    absMousePos(0.0f, 0.0f),
    normMousePos(0.0f, 0.0f),
    keyCode(Input::Key::InvalidKey),
    charCode(0),
    mouseButtonCode(Input::MouseButton::InvalidMouseButton)
{
}
inline
DisplayEvent::DisplayEvent(Code c, CoreGraphics::WindowId wnd) :
    code(c),
    windowId(wnd),
    absMousePos(0.0f, 0.0f),
    normMousePos(0.0f, 0.0f),
    keyCode(Input::Key::InvalidKey),
    charCode(0),
    mouseButtonCode(Input::MouseButton::InvalidMouseButton)
{
}
inline
DisplayEvent::DisplayEvent(Code c, const Math::vec2& absPos, const Math::vec2& normPos) :
    code(c),
    windowId(Ids::InvalidId32),
    absMousePos(absPos),
    normMousePos(normPos),
    keyCode(Input::Key::InvalidKey),
    charCode(0),
    mouseButtonCode(Input::MouseButton::InvalidMouseButton)
{
}
inline
DisplayEvent::DisplayEvent(Code c, Input::Key::Code k) :
    code(c),
    windowId(Ids::InvalidId32),
    absMousePos(0.0f, 0.0f),
    normMousePos(0.0f, 0.0f),
    keyCode(k),
    charCode(0),
    mouseButtonCode(Input::MouseButton::InvalidMouseButton)
{
}
inline
DisplayEvent::DisplayEvent(Code c, Input::Char chr) :
    code(c),
    windowId(Ids::InvalidId32),
    absMousePos(0.0f, 0.0f),
    normMousePos(0.0f, 0.0f),
    keyCode(Input::Key::InvalidKey),
    charCode(chr),
    mouseButtonCode(Input::MouseButton::InvalidMouseButton)
{
}
inline
DisplayEvent::DisplayEvent(Code c, Input::MouseButton::Code b, const Math::vec2& absPos, const Math::vec2& normPos) :
    code(c),
    windowId(Ids::InvalidId32),
    absMousePos(absPos),
    normMousePos(normPos),
    keyCode(Input::Key::InvalidKey),
    charCode(0),
    mouseButtonCode(b)
{
}
inline DisplayEvent::Code
DisplayEvent::GetEventCode() const
{
    return this->code;
}
inline CoreGraphics::WindowId
DisplayEvent::GetWindowId() const
{
    return this->windowId;
}
inline const Math::vec2&
DisplayEvent::GetAbsMousePos() const
{
    return this->absMousePos;
}
inline const Math::vec2&
DisplayEvent::GetNormMousePos() const
{
    return this->normMousePos;
}
inline Input::Key::Code
DisplayEvent::GetKey() const
{
    return this->keyCode;
}
inline Input::Char
DisplayEvent::GetChar() const
{
    return this->charCode;
}
inline Input::MouseButton::Code
DisplayEvent::GetMouseButton() const
{
    return this->mouseButtonCode;
}
}