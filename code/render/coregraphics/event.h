#pragma once
#include "ids/id.h"
#include "ids/idpool.h"
#include "coregraphics/barrier.h"
#include "coregraphics/commandbuffer.h"
#include "coregraphics/config.h"
#include "util/stringatom.h"
#ifdef CreateEvent
#pragma push_macro("CreateEvent")
#undef CreateEvent
#endif
namespace CoreGraphics
{
ID_24_8_TYPE(EventId);
struct EventCreateInfo
{
    Util::StringAtom name;
    bool createSignaled : 1;
    CoreGraphics::PipelineStage fromStage, toStage;
    Util::Array<TextureBarrierInfo> textures;
    Util::Array<BufferBarrierInfo> buffers;
};
EventId CreateEvent(const EventCreateInfo& info);
void DestroyEvent(const EventId id);
void EventSignal(const EventId id, const CoreGraphics::CmdBufferId buf, const CoreGraphics::PipelineStage stage);
void EventWait(
    const EventId id,
    const CoreGraphics::QueueType queue,
    const CoreGraphics::PipelineStage waitStage,
    const CoreGraphics::PipelineStage signalStage
    );
void EventWait(
    const EventId id,
    const CoreGraphics::CmdBufferId buf,
    const CoreGraphics::PipelineStage waitStage,
    const CoreGraphics::PipelineStage signalStage
);
void EventReset(const EventId id, const CoreGraphics::CmdBufferId buf, const CoreGraphics::PipelineStage stage);
void EventWaitAndReset(const EventId id, const CoreGraphics::CmdBufferId buf, const CoreGraphics::PipelineStage waitStage, const CoreGraphics::PipelineStage signalStage);
bool EventPoll(const EventId id);
void EventHostReset(const EventId id);
void EventHostSignal(const EventId id);
void EventHostWait(const EventId id);
}
#pragma pop_macro("CreateEvent")