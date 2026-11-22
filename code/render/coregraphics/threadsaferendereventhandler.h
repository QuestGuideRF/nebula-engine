#pragma once
#include "coregraphics/rendereventhandler.h"
#include "threading/safequeue.h"
namespace CoreGraphics
{
class ThreadSafeRenderEventHandler : public RenderEventHandler
{
    __DeclareClass(ThreadSafeRenderEventHandler);
public:
    ThreadSafeRenderEventHandler();
    virtual ~ThreadSafeRenderEventHandler();
    virtual bool PutEvent(const RenderEvent& event);
    void HandlePendingEvents();
protected:
    virtual bool HandleEvent(const RenderEvent& event);
    Threading::SafeQueue<RenderEvent> eventQueue;
    Util::Array<RenderEvent> tmpPendingEvents;
};
}