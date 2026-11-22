#pragma once
#include "core/types.h"
namespace CoreGraphics
{
class RenderEvent
{
public:
    enum Code
    {
        InvalidCode,
        DeviceOpen,
        DeviceClose,
        DeviceLost,
        DeviceRestored,
    };
    RenderEvent();
    RenderEvent(Code c);
    Code GetEventCode() const;
private:
    Code code;
};
inline
RenderEvent::RenderEvent() :
    code(InvalidCode)
{
}
inline
RenderEvent::RenderEvent(Code c) :
    code(c)
{
}
inline
RenderEvent::Code
RenderEvent::GetEventCode() const
{
    return this->code;
}
}