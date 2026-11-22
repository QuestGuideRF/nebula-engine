#pragma once
#include "core/refcounted.h"
#include "input/inputhandler.h"
namespace TBUI
{
class TBUIInputHandler : public Input::InputHandler
{
    __DeclareClass(TBUIInputHandler);
public:
    TBUIInputHandler();
    virtual ~TBUIInputHandler();
    virtual void BeginCapture();
    virtual void EndCapture();
    void OnBeginFrame() override;
protected:
    virtual bool OnEvent(const Input::InputEvent& inputEvent);
private:
    Util::Array<Input::InputEvent> inputEvents;
};
}