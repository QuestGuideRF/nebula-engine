#pragma once
#include "core/types.h"
namespace Timing
{
typedef double Time;
typedef int Tick;
inline Time
TicksToSeconds(Tick ticks)
{
    return ticks * 0.001;
}
inline Tick
SecondsToTicks(Time t)
{
    return Tick((t * 1000.0) + 0.5);
}
inline void
Sleep(Time t)
{
    n_sleep(t);
}
};