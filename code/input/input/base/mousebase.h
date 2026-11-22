#pragma once
#include "input/inputhandler.h"
#include "input/mousebutton.h"
namespace Base
{
class MouseBase : public Input::InputHandler
{
    __DeclareClass(MouseBase);
public:
    MouseBase();
    virtual ~MouseBase();
    virtual void BeginCapture();
    virtual void EndCapture();
    bool ButtonPressed(Input::MouseButton::Code btn) const;
    bool ButtonDown(Input::MouseButton::Code btn) const;
    bool ButtonUp(Input::MouseButton::Code btn) const;
    bool ButtonDoubleClicked(Input::MouseButton::Code btn) const;
    bool WheelForward() const;
    bool WheelBackward() const;
    const Math::vec2& GetPixelPosition() const;
    const Math::vec2& GetScreenPosition() const;
    const Math::vec2& GetMovement() const;
    virtual void LockToPosition(bool locked, Math::vec2& position);
    bool IsLocked() const;
protected:
    virtual void OnAttach();
    virtual void OnBeginFrame();
    virtual bool OnEvent(const Input::InputEvent& inputEvent);
    virtual void OnObtainCapture();
    virtual void OnReleaseCapture();
    virtual void OnReset();
private:
    void UpdateMousePositions(const Math::vec2& pixelPos, const Math::vec2& screenPos);
    struct ButtonState
    {
    public:
        ButtonState() : pressed(false), down(false), up(false), doubleClicked(false) {};
        bool pressed;
        bool down;
        bool up;
        bool doubleClicked;
    };
    Util::FixedArray<ButtonState> buttonStates;
    Math::vec2 beginFramePixelPosition;
    Math::vec2 beginFrameScreenPosition;
    Math::vec2 pixelPosition;
    Math::vec2 screenPosition;
    Math::vec2 movement;
    Math::vec2 mouseLockedPosition;
    bool wheelForward;
    bool wheelBackward;
    bool initialMouseMovement;
    bool mouseLocked;
    bool mouseWasLocked;
};
inline bool
MouseBase::ButtonPressed(Input::MouseButton::Code btn) const
{
    return this->buttonStates[btn].pressed;
}
inline bool
MouseBase::ButtonDown(Input::MouseButton::Code btn) const
{
    return this->buttonStates[btn].down;
}
inline bool
MouseBase::ButtonUp(Input::MouseButton::Code btn) const
{
    return this->buttonStates[btn].up;
}
inline bool
MouseBase::ButtonDoubleClicked(Input::MouseButton::Code btn) const
{
    return this->buttonStates[btn].doubleClicked;
}
inline const Math::vec2&
MouseBase::GetPixelPosition() const
{
    return this->pixelPosition;
}
inline const Math::vec2&
MouseBase::GetScreenPosition() const
{
    return this->screenPosition;
}
inline const Math::vec2&
MouseBase::GetMovement() const
{
    return this->movement;
}
inline bool
MouseBase::WheelForward() const
{
    return this->wheelForward;
}
inline bool
MouseBase::WheelBackward() const
{
    return this->wheelBackward;
}
inline void
MouseBase::LockToPosition(bool locked, Math::vec2& position)
{
    mouseLockedPosition = position;
    mouseLocked = locked;
    mouseWasLocked = locked;
}
inline bool
MouseBase::IsLocked() const
{
    return mouseLocked;
}
}