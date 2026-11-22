#pragma once
#include "core/types.h"
#include "math/vec4.h"
#include "coreanimation/sampletype.h"
namespace CoreAnimation
{
struct NEBULA_ALIGN16 AnimSampleMixInfo
{
    SampleType::Code sampleType;
    float mixWeight;
    Math::vec4 velocityScale;
};
}