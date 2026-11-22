#pragma once
#include "core/types.h"
#include "input/key.h"
#include "input/char.h"
#include "input/mousebutton.h"
#include "math/vec2.h"
namespace Input
{
class InputEvent
{
public:
    enum Type
    {
        InvalidType = 0,
        AppObtainFocus,
        AppLoseFocus,
        AppClose,
        Reset,
        KeyDown,
        KeyUp,
        Character,
        MouseMove,
        RawMouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseButtonDoubleClick,
        MouseWheelForward,
        MouseWheelBackward,
        BeginMouseCapture,
        EndMouseCapture,
        BeginKeyboardCapture,
        EndKeyboardCapture,
        GamePadButtonDown,
        GamePadButtonUp,
    };
    InputEvent();
    void SetType(Type t);
    Type GetType() const;
    void SetKey(Key::Code key);
    Key::Code GetKey() const;
    void SetChar(Char chr);
    Char GetChar() const;
    void SetMouseButton(MouseButton::Code button);
    MouseButton::Code GetMouseButton() const;
    void SetDeviceIndex(IndexT i);
    IndexT GetDeviceIndex() const;
    void SetAbsMousePos(const Math::vec2& p);
    const Math::vec2& GetAbsMousePos() const;
    void SetNormMousePos(const Math::vec2& p);
    const Math::vec2& GetNormMousePos() const;
    static const char* TypeToString(Type t);
private:
    Type type;
    Key::Code keyCode;
    Char character;
    IndexT deviceIndex;
    MouseButton::Code mouseButton;
    Math::vec2 absMousePos;
    Math::vec2 normMousePos;
};
inline
InputEvent::InputEvent() :
    type(InvalidType),
    keyCode(Key::InvalidKey),
    character(0),
    deviceIndex(0),
    mouseButton(MouseButton::InvalidMouseButton)
{
}
inline void
InputEvent::SetType(Type t)
{
    this->type = t;
}
inline InputEvent::Type
InputEvent::GetType() const
{
    return this->type;
}
inline void
InputEvent::SetKey(Key::Code key)
{
    this->keyCode = key;
}
inline Key::Code
InputEvent::GetKey() const
{
    return this->keyCode;
}
inline void
InputEvent::SetChar(Char chr)
{
    this->character = chr;
}
inline Char
InputEvent::GetChar() const
{
    return this->character;
}
inline void
InputEvent::SetMouseButton(MouseButton::Code btn)
{
    this->mouseButton = btn;
}
inline MouseButton::Code
InputEvent::GetMouseButton() const
{
    return this->mouseButton;
}
inline void
InputEvent::SetAbsMousePos(const Math::vec2& p)
{
    this->absMousePos = p;
}
inline const Math::vec2&
InputEvent::GetAbsMousePos() const
{
    return this->absMousePos;
}
inline void
InputEvent::SetNormMousePos(const Math::vec2& p)
{
    this->normMousePos = p;
}
inline const Math::vec2&
InputEvent::GetNormMousePos() const
{
    return this->normMousePos;
}
inline void
InputEvent::SetDeviceIndex(IndexT i)
{
    this->deviceIndex = i;
}
inline IndexT
InputEvent::GetDeviceIndex() const
{
    return this->deviceIndex;
}
inline const char*
InputEvent::TypeToString(Type t)
{
    switch (t)
    {
        case InvalidType:               return "InvalidTyp";
        case AppObtainFocus:            return "AppObtainFocus";
        case AppLoseFocus:              return "AppLoseFocus";
        case AppClose:                  return "AppClose";
        case Reset:                     return "Reset";
        case KeyDown:                   return "KeyDown";
        case KeyUp:                     return "KeyUp";
        case Character:                 return "Character";
        case MouseMove:                 return "MouseMove";
        case RawMouseMove:              return "RawMouseMove";
        case MouseButtonDown:           return "MouseButtonDown";
        case MouseButtonUp:             return "MouseButtonUp";
        case MouseButtonDoubleClick:    return "MouseButtonDoubleClick";
        case MouseWheelForward:         return "MouseWheelForward";
        case MouseWheelBackward:        return "MouseWheelBackward";
        case BeginMouseCapture:         return "BeginMouseCapture";
        case EndMouseCapture:           return "EndMouseCapture";
        case BeginKeyboardCapture:      return "BeginKeyboardCapture";
        case EndKeyboardCapture:        return "EndKeyboardCapture";
        case GamePadButtonDown:         return "GamePadButtonDown";
        case GamePadButtonUp:           return "GamePadButtonUp";
        default:
            n_error("InputEvent::TypeToString(): неверный тип!\n");
            return "<INVALID>";
    }
}
}