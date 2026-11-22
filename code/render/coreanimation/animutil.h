#pragma once
#include "core/types.h"
#include "coreanimation/animclip.h"
#include "coreanimation/sampletype.h"
#include "coreanimation/animresource.h"
#include "coreanimation/animsamplebuffer.h"
#include "characters/characterjointmask.h"
#include "jobs/job.h"
namespace CoreAnimation
{
class AnimUtil
{
public:
    static void Sample(const AnimResourceId& animResource, IndexT clipIndex, SampleType::Code sampleType, Timing::Tick time, float timeFactor, const Ptr<AnimSampleBuffer>& result);
    static Ptr<Jobs::Job> CreateSampleJob(const AnimResourceId& animResource, IndexT clipIndex, SampleType::Code sampleType, Timing::Tick time, float timeFactor, const Characters::CharacterJointMask* mask, const Ptr<AnimSampleBuffer>& result);
    static Ptr<Jobs::Job> CreateSampleAndMixJob(const AnimResourceId& animResource, IndexT clipIndex, SampleType::Code sampleType, Timing::Tick time, float timeFactor, const Characters::CharacterJointMask* mask, float mixWeight, const Ptr<AnimSampleBuffer>& mixIn, const Ptr<AnimSampleBuffer>& result);
    static IndexT ClampKeyIndex(IndexT keyIndex, const AnimClip& clip);
    static Timing::Tick InbetweenTicks(Timing::Tick sampleTime, const AnimClip& clip);
};
}