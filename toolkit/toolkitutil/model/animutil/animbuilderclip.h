#pragma once
#include "core/types.h"
#include "util/stringatom.h"
#include "coreanimation/infinitytype.h"
#include "coreanimation/animevent.h"
#include "timing/time.h"
#include "model/animutil/animbuildercurve.h"
namespace ToolkitUtil
{
class AnimBuilderClip
{
public:
    AnimBuilderClip();
    void SetName(const Util::StringAtom& n);
    const Util::StringAtom& GetName() const;
    uint firstCurveOffset;
    uint numCurves;
    uint firstEventOffset;
    uint numEvents;
    uint firstVelocityCurveOffset;
    uint numVelocityCurves;
    Timing::Tick duration;
private:
    Util::StringAtom name;
};
inline void
AnimBuilderClip::SetName(const Util::StringAtom& n)
{
    this->name = n;
}
inline const Util::StringAtom&
AnimBuilderClip::GetName() const
{
    return this->name;
}
}