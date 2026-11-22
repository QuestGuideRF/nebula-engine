#pragma once
#include "core/refcounted.h"
#include "input/inputhandler.h"
namespace Dynui
{
class ImguiInputHandler : public Input::InputHandler
{
    __DeclareClass(ImguiInputHandler);
public:
    ImguiInputHandler();
    virtual ~ImguiInputHandler();
    virtual void BeginCapture();
    virtual void EndCapture();
    void OnBeginFrame() override;
protected:
    virtual bool OnEvent(const Input::InputEvent& inputEvent);
private:
    Util::Array<Input::InputEvent> inputEvents;
};
}