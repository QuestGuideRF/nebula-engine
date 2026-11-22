#pragma once
#include "input/inputhandler.h"
#include "input/key.h"
namespace Base
{
class KeyboardBase : public Input::InputHandler
{
    __DeclareClass(KeyboardBase);
public:
    KeyboardBase();
    virtual ~KeyboardBase();
    virtual void BeginCapture();
    virtual void EndCapture();
    void SetKeyDown(Input::Key::Code keyCode);
    void SetKeyUp(Input::Key::Code keyCode);
    bool KeyPressed(Input::Key::Code keyCode) const;
    bool KeyDown(Input::Key::Code keyCode) const;
    bool KeyUp(Input::Key::Code keyCode) const;
    const Util::String& GetCharInput() const;
protected:
    virtual void OnAttach();
    virtual void OnBeginFrame();
    virtual bool OnEvent(const Input::InputEvent& inputEvent);
    virtual void OnObtainCapture();
    virtual void OnReleaseCapture();
    virtual void OnReset();
private:
    class KeyState
    {
    public:
        KeyState() : pressed(false), down(false), up(false) {};
        bool pressed;
        bool down;
        bool up;
    };
    Util::FixedArray<KeyState> keyStates;
    Util::FixedArray<KeyState> nextKeyStates;
    Util::String charInput;
};
inline void
KeyboardBase::SetKeyDown(Input::Key::Code keyCode)
{
    KeyState& keyState = this->keyStates[keyCode];
    if (!keyState.pressed)
    {
        keyState.down = true;
        keyState.pressed = true;
    }
}
inline void
KeyboardBase::SetKeyUp(Input::Key::Code keyCode)
{
    this->keyStates[keyCode].up = true;
}
inline bool
KeyboardBase::KeyPressed(Input::Key::Code keyCode) const
{
    return this->keyStates[keyCode].pressed;
}
inline bool
KeyboardBase::KeyDown(Input::Key::Code keyCode) const
{
    return this->keyStates[keyCode].down;
}
inline bool
KeyboardBase::KeyUp(Input::Key::Code keyCode) const
{
    return this->keyStates[keyCode].up;
}
inline const Util::String&
KeyboardBase::GetCharInput() const
{
    return this->charInput;
}
}