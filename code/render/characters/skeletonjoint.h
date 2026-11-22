#pragma once
#include "core/types.h"
namespace Characters
{
struct SkeletonJobJoint
{
    SkeletonJobJoint() :
        varTranslationX(0.0f),
        varTranslationY(0.0f),
        varTranslationZ(0.0f),
        varTranslationW(0.0f),
        varScaleX(1.0f),
        varScaleY(1.0f),
        varScaleZ(1.0f),
        parentJointIndex(InvalidIndex)
    { };
    float varTranslationX, varTranslationY, varTranslationZ, varTranslationW;
    float varScaleX, varScaleY, varScaleZ;
    int parentJointIndex;
};
}