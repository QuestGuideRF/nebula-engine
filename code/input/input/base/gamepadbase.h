#pragma once
#include "input/inputhandler.h"
#include "threading/criticalsection.h"
#include "threading/thread.h"
#include "math/mat4.h"
#define MAX_GAMEPADS 4
namespace Base
{
class GamePadBase : public Input::InputHandler
{
    __DeclareClass(GamePadBase);
public:
    enum Button
    {
        DPadUpButton = 0,
        DPadDownButton,
        DPadLeftButton,
        DPadRightButton,
        StartButton,
        BackButton,
        LeftThumbButton,
        RightThumbButton,
        LeftShoulderButton,
        RightShoulderButton,
        AButton,
        BButton,
        XButton,
        YButton,
        HomeButton,
        PlusButton,
        MinusButton,
        OneButton,
        TwoButton,
        ZButton,
        CButton,
        SelectButton,
        NumButtons,
        InvalidButton,
        CrossButton = AButton,
        CircleButton = BButton,
        SquareButton = XButton,
        TriangleButton = YButton,
        L1Button = LeftShoulderButton,
        R1Button = RightShoulderButton,
        L3Button = LeftThumbButton,
        R3Button = RightThumbButton,
    };
    enum Axis
    {
        LeftTriggerAxis = 0,
        RightTriggerAxis,
        LeftThumbXAxis,
        LeftThumbYAxis,
        RightThumbXAxis,
        RightThumbYAxis,
        NumAxes,
        InvalidAxis,
    };
    GamePadBase();
    virtual ~GamePadBase();
    static Util::String ButtonAsString(Button btn);
    static Util::String AxisAsString(Axis a);
    bool IsConnected() const;
    void SetIndex(IndexT i);
    IndexT GetIndex() const;
    static SizeT GetMaxNumControllers();
    bool ButtonPressed(Button btn) const;
    bool ButtonDown(Button btn) const;
    bool ButtonUp(Button btn) const;
    float GetAxisValue(Axis axis) const;
    bool HasTransform() const;
    const Math::mat4 & GetTransform() const;
    void SetLowFrequencyVibrator(float f);
    float GetLowFrequencyVibrator() const;
    void SetHighFrequencyVibrator(float f);
    float GetHighFrequencyVibrator() const;
    Util::Array<Input::InputEvent> GetStateAsInputEvents() const;
protected:
    virtual void OnAttach();
    virtual void OnReset();
    struct ButtonState
    {
    public:
        ButtonState() : pressed(false), down(false), up(false) {};
        bool pressed;
        bool down;
        bool up;
    };
    IndexT index;
    bool isConnected;
    Util::FixedArray<ButtonState> buttonStates;
    Util::FixedArray<float> axisValues;
    bool vibratorsDirty;
    bool hasTransform;
    float lowFreqVibrator;
    float highFreqVibrator;
    Threading::CriticalSection critSect;
    Threading::ThreadId creatorThreadId;
    Math::mat4 transform;
};
inline bool
GamePadBase::IsConnected() const
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "IsConnected нельзя вызвать ни из какого потока, кроме потока создателя!");
    return this->isConnected;
}
inline bool
GamePadBase::HasTransform() const
{
    return this->hasTransform;
}
inline
const Math::mat4 &
GamePadBase::GetTransform() const
{
    return this->transform;
}
inline void
GamePadBase::SetIndex(IndexT i)
{
    n_assert(i < this->GetMaxNumControllers());
    this->index = i;
}
inline IndexT
GamePadBase::GetIndex() const
{
    return this->index;
}
inline SizeT
GamePadBase::GetMaxNumControllers()
{
    return MAX_GAMEPADS;
}
inline bool
GamePadBase::ButtonPressed(Button btn) const
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "ButtonPressed нельзя вызвать ни из какого потока, кроме потока создателя!");
    return this->buttonStates[btn].pressed;
}
inline bool
GamePadBase::ButtonDown(Button btn) const
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "ButtonDown нельзя вызвать ни из какого потока, кроме потока создателя!");
    return this->buttonStates[btn].down;
}
inline bool
GamePadBase::ButtonUp(Button btn) const
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "ButtonUp нельзя вызвать ни из какого потока, кроме потока создателя!");
    return this->buttonStates[btn].up;
}
inline float
GamePadBase::GetAxisValue(Axis axis) const
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "GetAxisValue нельзя вызывать ни из какого потока, кроме потока создателя!");
    return this->axisValues[axis];
}
inline void
GamePadBase::SetLowFrequencyVibrator(float f)
{
    critSect.Enter();
    this->vibratorsDirty = true;
    this->lowFreqVibrator = Math::clamp(f, 0.0f, 1.0f);
    critSect.Leave();
}
inline float
GamePadBase::GetLowFrequencyVibrator() const
{
    critSect.Enter();
    float result = this->lowFreqVibrator;
    critSect.Leave();
    return result;
}
inline void
GamePadBase::SetHighFrequencyVibrator(float f)
{
    critSect.Enter();
    this->vibratorsDirty = true;
    this->highFreqVibrator = Math::clamp(f, 0.0f, 1.0f);;
    critSect.Leave();
}
inline float
GamePadBase::GetHighFrequencyVibrator() const
{
    critSect.Enter();
    float result = this->highFreqVibrator;
    critSect.Leave();
    return result;
}
}