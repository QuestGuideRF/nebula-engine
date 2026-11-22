#pragma once
#include "core/refcounted.h"
#include "input/inputpriority.h"
#include "input/inputevent.h"
#include "threading/criticalsection.h"
#include "threading/thread.h"
namespace Input
{
    class Mouse;
    class Keyboard;
    class GamePad;
    class InputHandler;
}
namespace Base
{
class InputServerBase : public Core::RefCounted
{
    __DeclareClass(InputServerBase);
public:
    InputServerBase();
    virtual ~InputServerBase();
    void SetMaxNumLocalPlayers(SizeT maxNumLocalPlayers);
    SizeT GetMaxNumLocalPlayers() const;
    virtual void Open();
    virtual void Close();
    bool IsOpen() const;
    void SetQuitRequested(bool b);
    bool IsQuitRequested() const;
    void Reset();
    const Ptr<Input::Keyboard>& GetDefaultKeyboard() const;
    const Ptr<Input::Mouse>& GetDefaultMouse() const;
    Ptr<Input::GamePad> GetDefaultGamePad(IndexT playerIndex) const;
    void AttachInputHandler(Input::InputPriority::Code pri, const Ptr<Input::InputHandler>& inputHandler);
    void RemoveInputHandler(const Ptr<Input::InputHandler>& inputHandler);
    void AddGamePad(const Ptr<Input::GamePad> pad, IndexT playerIndex);
    virtual void BeginFrame();
    virtual void OnFrame();
    void EndFrame();
    void PutEvent(const Input::InputEvent& ie);
    void ClearMouseCapture();
    void ClearKeyboardCapture();
    void ClearCapture();
    const Ptr<Input::InputHandler>& GetMouseCaptureHandler() const;
    const Ptr<Input::InputHandler>& GetKeyboardCaptureHandler() const;
    void ObtainMouseCapture(const Ptr<Input::InputHandler>& inputHandler);
    void ReleaseMouseCapture(const Ptr<Input::InputHandler>& inputHandler);
    void ObtainKeyboardCapture(const Ptr<Input::InputHandler>& inputHandler);
    void ReleaseKeyboardCapture(const Ptr<Input::InputHandler>& inputHandler);
    bool HasFocus() const;
protected:
    bool isOpen;
    bool inBeginFrame;
    bool isQuitRequested;
    bool hasFocus;
    int inputHandlersLockCount;
    SizeT maxNumLocalPlayers;
    Util::Array<Util::KeyValuePair<Input::InputPriority::Code,Ptr<Input::InputHandler> > > inputHandlers;
    Ptr<Input::InputHandler> mouseCaptureHandler;
    Ptr<Input::InputHandler> keyboardCaptureHandler;
    Ptr<Input::Keyboard> defaultKeyboard;
    Ptr<Input::Mouse> defaultMouse;
    Util::FixedArray<Ptr<Input::GamePad> > defaultGamePad;
    Threading::CriticalSection critSect;
    Threading::ThreadId creatorThreadId;
};
inline bool
InputServerBase::IsOpen() const
{
    return this->isOpen;
}
inline SizeT
InputServerBase::GetMaxNumLocalPlayers() const
{
    return this->maxNumLocalPlayers;
}
inline void
InputServerBase::SetQuitRequested(bool b)
{
    this->isQuitRequested = b;
}
inline bool
InputServerBase::IsQuitRequested() const
{
    return this->isQuitRequested;
}
inline const Ptr<Input::Keyboard>&
InputServerBase::GetDefaultKeyboard() const
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "GetDefaultKeyboard нельзя вызвать ни из какого потока, кроме потока создателя!");
    return this->defaultKeyboard;
}
inline const Ptr<Input::Mouse>&
InputServerBase::GetDefaultMouse() const
{
    n_assert2(creatorThreadId == Threading::Thread::GetMyThreadId(), "GetDefaultMouse нельзя вызвать ни из какого потока, кроме потока создателя!");
    return this->defaultMouse;
}
inline bool
InputServerBase::HasFocus() const
{
    return hasFocus;
}
}