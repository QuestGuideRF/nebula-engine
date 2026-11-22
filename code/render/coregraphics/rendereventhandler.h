#pragma once
#include "core/refcounted.h"
#include "coregraphics/renderevent.h"
namespace CoreGraphics
{
class RenderEventHandler : public Core::RefCounted
{
    __DeclareClass(RenderEventHandler);
public:
    RenderEventHandler();
    virtual ~RenderEventHandler();
    virtual void OnAttach();
    virtual void OnRemove();
    virtual bool PutEvent(const RenderEvent& event);
protected:
    virtual bool HandleEvent(const RenderEvent& event);
};
}