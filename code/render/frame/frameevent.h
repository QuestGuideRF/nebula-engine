#pragma once
#include "frameop.h"
#include "coregraphics/event.h"
namespace Frame
{
class FrameEvent : public FrameOp
{
public:
    enum Action
    {
        Set,
        Reset,
        Wait
    };
    FrameEvent();
    virtual ~FrameEvent();
    void Discard();
    void Run(const IndexT frameIndex);
    Util::Array<Action> actions;
    CoreGraphics::EventId event;
    CoreGraphics::PipelineStage dependency;
    CoreGraphics::QueueType queueType;
};
}