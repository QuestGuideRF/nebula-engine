#pragma once
#include "coreanimation/infinitytype.h"
#include "coreanimation/animcurve.h"
#include "coreanimation/animevent.h"
#include "util/stringatom.h"
#include "timing/time.h"
#include "util/dictionary.h"
#include "util/fixedarray.h"
namespace CoreAnimation
{
class AnimClip
{
public:
    AnimClip();
    void SetName(const Util::StringAtom& n);
    const Util::StringAtom& GetName() const;
    Util::StringAtom name;
    SizeT numCurves;
    IndexT firstCurve;
    SizeT numEvents;
    IndexT firstEvent;
    SizeT numVelocityCurves;
    IndexT firstVelocityCurve;
    Timing::Tick duration;
    Util::Dictionary<Util::StringAtom, IndexT> eventIndexMap;
};
inline void
AnimClip::SetName(const Util::StringAtom& n)
{
    this->name = n;
}
inline const Util::StringAtom&
AnimClip::GetName() const
{
    return this->name;
}
}