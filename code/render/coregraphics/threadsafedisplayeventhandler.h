#pragma once
#include "coregraphics/displayeventhandler.h"
#include "threading/safequeue.h"
namespace CoreGraphics
{
class ThreadSafeDisplayEventHandler : public DisplayEventHandler
{
    __DeclareClass(ThreadSafeDisplayEventHandler);
public:
    ThreadSafeDisplayEventHandler();
    virtual ~ThreadSafeDisplayEventHandler();
    virtual bool PutEvent(const DisplayEvent& event);
    void HandlePendingEvents();
protected:
    virtual bool HandleEvent(const DisplayEvent& event);
    Threading::SafeQueue<DisplayEvent> eventQueue;
    Util::Array<DisplayEvent> tmpPendingEvents;
};
}