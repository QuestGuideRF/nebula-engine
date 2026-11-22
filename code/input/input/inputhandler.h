#pragma once
#include "core/refcounted.h"
#include "input/inputevent.h"
namespace Base
{
    class InputServerBase;
}
namespace Input
{
class InputHandler : public Core::RefCounted
{
    __DeclareClass(InputHandler);
public:
    InputHandler();
    virtual ~InputHandler();
    bool IsAttached() const;
    virtual void BeginCapture();
    virtual void EndCapture();
    bool IsCapturing() const;
protected:
    friend class Base::InputServerBase;
    virtual void OnAttach();
    virtual void OnRemove();
    virtual void OnBeginFrame();
    virtual void OnEndFrame();
    virtual void OnObtainCapture();
    virtual void OnReleaseCapture();
    virtual bool OnEvent(const InputEvent& inputEvent);
    virtual void OnReset();
    bool isAttached;
    bool isCapturing;
};
inline bool
InputHandler::IsAttached() const
{
    return this->isAttached;
}
inline bool
InputHandler::IsCapturing() const
{
    return this->isCapturing;
}
}