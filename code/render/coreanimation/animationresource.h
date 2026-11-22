#pragma once
#include "resources/resourceid.h"
#include "ids/idallocator.h"
#include "util/fixedarray.h"
#include "animation.h"
namespace CoreAnimation
{
RESOURCE_ID_TYPE(AnimationResourceId);
const AnimationId AnimationResourceGetAnimation(const AnimationResourceId id, IndexT index);
void DestroyAnimationResource(const AnimationResourceId id);
typedef Ids::IdAllocator<
    Util::FixedArray<AnimationId>
> AnimationResourceAllocator;
extern AnimationResourceAllocator animationResourceAllocator;
}