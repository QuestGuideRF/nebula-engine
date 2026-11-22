#pragma once
#include "core/refcounted.h"
#include "coregraphics/displayevent.h"
namespace CoreGraphics
{
class DisplayEventHandler : public Core::RefCounted
{
    __DeclareClass(DisplayEventHandler);
public:
    DisplayEventHandler();
    virtual ~DisplayEventHandler();
    virtual void OnAttach();
    virtual void OnRemove();
    virtual bool PutEvent(const DisplayEvent& event);
protected:
    virtual bool HandleEvent(const DisplayEvent& event);
};
}