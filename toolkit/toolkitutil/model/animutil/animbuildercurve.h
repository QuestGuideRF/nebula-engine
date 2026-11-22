#pragma once
#include "core/types.h"
#include "coreanimation/curvetype.h"
#include "coreanimation/infinitytype.h"
#include "util/fixedarray.h"
#include "math/vec4.h"
namespace ToolkitUtil
{
class AnimBuilderCurve
{
public:
    AnimBuilderCurve();
    uint firstKeyOffset;
    uint firstTimeOffset;
    uint numKeys;
    CoreAnimation::CurveType::Code curveType;
    CoreAnimation::InfinityType::Code preInfinityType;
    CoreAnimation::InfinityType::Code postInfinityType;
};
}