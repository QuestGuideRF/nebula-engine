#pragma once
#include "resources/resourceid.h"
#include "ids/idallocator.h"
#include "util/fixedarray.h"
#include "skeleton.h"
namespace Characters
{
RESOURCE_ID_TYPE(SkeletonResourceId);
const SkeletonId SkeletonResourceGetSkeleton(const SkeletonResourceId id, IndexT index);
void DestroySkeletonResource(const SkeletonResourceId id);
typedef Ids::IdAllocator<
    Util::FixedArray<SkeletonId>
> SkeletonResourceAllocator;
extern SkeletonResourceAllocator skeletonResourceAllocator;
}