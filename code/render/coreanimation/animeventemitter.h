#pragma once
#include "util/array.h"
#include "coreanimation/animclip.h"
#include "coreanimation/animevent.h"
namespace CoreAnimation
{
class AnimEventEmitter
{
public:
    static Util::Array<AnimEvent> EmitAnimEvents(const AnimClip& clip, const Util::FixedArray<AnimEvent>& events, Timing::Tick start, Timing::Tick end, bool isInfinite);
};
}