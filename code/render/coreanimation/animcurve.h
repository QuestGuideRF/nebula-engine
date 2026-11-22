#pragma once
#include "core/types.h"
#include "coreanimation/curvetype.h"
#include "coreanimation/infinitytype.h"
#include "math/vec4.h"
namespace CoreAnimation
{
class AnimCurve
{
public:
    AnimCurve();
    uint firstIntervalOffset;
    uint numIntervals;
    CoreAnimation::InfinityType::Code preInfinityType;
    CoreAnimation::InfinityType::Code postInfinityType;
    CurveType::Code curveType;
};
inline
AnimCurve::AnimCurve()
    : firstIntervalOffset(0)
    , numIntervals(0)
    , preInfinityType(CoreAnimation::InfinityType::InvalidInfinityType)
    , postInfinityType(CoreAnimation::InfinityType::InvalidInfinityType)
    , curveType(CoreAnimation::CurveType::InvalidCurveType)
{
}
}