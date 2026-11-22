#pragma once
#include "util/stringatom.h"
#include "timing/time.h"
namespace CoreAnimation
{
class AnimEvent
{
public:
    AnimEvent();
    AnimEvent(const Util::StringAtom& name, Timing::Tick time);
    AnimEvent(const Util::StringAtom& name, const Util::StringAtom& category, Timing::Tick time);
    friend bool operator==(const AnimEvent& a, const AnimEvent& b);
    friend bool operator!=(const AnimEvent& a, const AnimEvent& b);
    friend bool operator<(const AnimEvent& a, const AnimEvent& b);
    friend bool operator>(const AnimEvent& a, const AnimEvent& b);
    friend bool operator<=(const AnimEvent& a, const AnimEvent& b);
    friend bool operator>=(const AnimEvent& a, const AnimEvent& b);
    Util::StringAtom name, category;
    Timing::Tick time;
};
inline
AnimEvent::AnimEvent() :
    name(nullptr),
    category(nullptr),
    time(0)
{
}
inline
AnimEvent::AnimEvent(const Util::StringAtom& n, Timing::Tick t) :
    name(n),
    time(t)
{
}
inline
AnimEvent::AnimEvent(const Util::StringAtom& n, const Util::StringAtom& c, Timing::Tick t) :
    name(n),
    category(c),
    time(t)
{
}
inline bool
operator==(const AnimEvent& a, const AnimEvent& b)
{
    return (a.time == b.time);
}
inline bool
operator!=(const AnimEvent& a, const AnimEvent& b)
{
    return (a.time != b.time);
}
inline bool
operator<(const AnimEvent& a, const AnimEvent& b)
{
    return (a.time < b.time);
}
inline bool
operator>(const AnimEvent& a, const AnimEvent& b)
{
    return (a.time > b.time);
}
inline bool
operator<=(const AnimEvent& a, const AnimEvent& b)
{
    return (a.time <= b.time);
}
inline bool
operator>=(const AnimEvent& a, const AnimEvent& b)
{
    return (a.time >= b.time);
}
}