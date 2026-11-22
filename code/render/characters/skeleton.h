#pragma once
#include "resources/resourceid.h"
#include "util/fixedarray.h"
#include "math/vector.h"
namespace Characters
{
ID_24_8_TYPE(SkeletonId);
struct CharacterJoint
{
    IndexT parentJointIndex;
    const CharacterJoint* parentJoint;
#if NEBULA_DEBUG
    Util::StringAtom name;
#endif
};
struct SkeletonCreateInfo
{
    Util::FixedArray<CharacterJoint> joints;
    Util::FixedArray<Math::mat4> bindPoses;
    Util::HashTable<Util::StringAtom, IndexT> jointIndexMap;
    Util::FixedArray<Math::vec4> idleSamples;
};
const SkeletonId CreateSkeleton(const SkeletonCreateInfo& info);
void DestroySkeleton(const SkeletonId id);
const SizeT SkeletonGetNumJoints(const SkeletonId id);
const Util::FixedArray<CharacterJoint>& SkeletonGetJoints(const SkeletonId id);
const Util::FixedArray<Math::mat4>& SkeletonGetBindPose(const SkeletonId id);
const IndexT SkeletonGetJointIndex(const SkeletonId id, const Util::StringAtom& name);
const Util::FixedArray<Math::vec4>& SkeletonGetIdleSamples(const SkeletonId id);
enum
{
    Skeleton_Joints,
    Skeleton_BindPose,
    Skeleton_JointNameMap,
    Skeleton_IdleSamples
};
typedef Ids::IdAllocator<
    Util::FixedArray<CharacterJoint>,
    Util::FixedArray<Math::mat4>,
    Util::HashTable<Util::StringAtom, IndexT>,
    Util::FixedArray<Math::vec4>
> SkeletonAllocator;
extern SkeletonAllocator skeletonAllocator;
}