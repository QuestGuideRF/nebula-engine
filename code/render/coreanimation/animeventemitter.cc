//------------------------------------------------------------------------------
//  animeventemitter.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coreanimation/animeventemitter.h"

namespace CoreAnimation
{
//------------------------------------------------------------------------------
/**
    собирает все аниме-события из данного клипа
*/
Util::Array<AnimEvent> 
CoreAnimation::AnimEventEmitter::EmitAnimEvents(const AnimClip& clip, const Util::FixedArray<AnimEvent>& events, Timing::Tick start, Timing::Tick end, bool isInfinite)
{
    Util::Array<AnimEvent> ret;

    if (start <= end)
    {
        for (int i = 0; i < clip.numEvents; i++)
        {
            const AnimEvent& event = events[clip.firstEvent + i];
            if (start <= event.time && end >= event.time)
                ret.Append(event);
        }
    }
    else
    {
        // from startCheckTime till end of clip
        Timing::Tick duration = clip.duration;
        ret.AppendArray(AnimEventEmitter::EmitAnimEvents(clip, events, start, duration, false));
        if (isInfinite)
        {
            // from start of clip till endCheckTime
            ret.AppendArray(AnimEventEmitter::EmitAnimEvents(clip, events, 0, end, false));
        }
    }
    return ret;
}
}
