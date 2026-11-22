#pragma once
#include "core/refcounted.h"
#include "coreanimation/animresource.h"
#include "coreanimation/animsamplebuffer.h"
#include "jobs/jobport.h"
#include "math/vector.h"
#include "math/point.h"
#include "animation/animjobenqueuemode.h"
#include "characters/characterjointmask.h"
namespace Animation
{
class AnimSequencer;
class AnimEventInfo;
class AnimJob : public Core::RefCounted
{
    __DeclareClass(AnimJob);
public:
    AnimJob();
    virtual ~AnimJob();
    void SetName(const Util::StringAtom& id);
    const Util::StringAtom& GetName() const;
    bool IsAttachedToSequencer() const;
    bool IsActive(Timing::Tick time) const;
    bool IsPending(Timing::Tick time) const;
    bool IsStoppingOrExpired(Timing::Tick time) const;
    bool IsExpired(Timing::Tick time) const;
    bool IsPaused() const;
    void SetTrackIndex(IndexT trackIndex);
    IndexT GetTrackIndex() const;
    void SetEnqueueMode(AnimJobEnqueueMode::Code enqueueMode);
    AnimJobEnqueueMode::Code GetEnqueueMode() const;
    void SetExclusiveTag(IndexT id);
    IndexT GetExclusiveTag() const;
    void SetStartTime(Timing::Tick time);
    Timing::Tick GetStartTime() const;
    void SetDuration(Timing::Tick time);
    Timing::Tick GetDuration() const;
    bool IsInfinite() const;
    void SetFadeInTime(Timing::Tick fadeInTime);
    Timing::Tick GetFadeInTime() const;
    void SetFadeOutTime(Timing::Tick fadeOutTime);
    Timing::Tick GetFadeOutTime() const;
    void SetTime(Timing::Tick time);
    Timing::Tick GetTime() const;
    void SetTimeFactor(float timeFactor);
    float GetTimeFactor() const;
    void SetTimeOffset(Timing::Tick timeOffset);
    Timing::Tick GetTimeOffset() const;
    void SetBlendWeight(float w);
    float GetBlendWeight() const;
    void SetMask(const Characters::CharacterJointMask* mask);
    const Characters::CharacterJointMask* GetMask() const;
    Timing::Tick GetAbsoluteStartTime() const;
    Timing::Tick GetAbsoluteEndTime() const;
    Timing::Tick GetAbsoluteStopTime() const;
protected:
    friend class AnimSequencer;
    void SetBaseTime(Timing::Tick time);
    Timing::Tick GetBaseTime() const;
    virtual void OnAttachedToSequencer(const AnimSequencer& animSequencer);
    virtual void OnRemoveFromSequencer();
    float ComputeBlendWeight(Timing::Tick relEvalTime) const;
    void FixFadeTimes();
    virtual Ptr<Jobs::Job> CreateEvaluationJob(Timing::Tick time, const Ptr<CoreAnimation::AnimSampleBuffer>& mixIn, const Ptr<CoreAnimation::AnimSampleBuffer>& result);
    virtual Util::Array<AnimEventInfo> EmitAnimEvents(Timing::Tick startTime, Timing::Tick endTime, const Util::String& optionalCatgeory);
    virtual void UpdateTimes(Timing::Tick time);
    virtual void Stop(Timing::Tick time);
    virtual void Pause(bool pause);
    const AnimSequencer* animSequencer;
    const Characters::CharacterJointMask* mask;
    Util::StringAtom name;
    AnimJobEnqueueMode::Code enqueueMode;
    IndexT trackIndex;
    IndexT exclusiveTag;
    Timing::Tick baseTime;
    Timing::Tick startTime;
    Timing::Tick duration;
    Timing::Tick fadeInTime;
    Timing::Tick fadeOutTime;
    Timing::Tick curRelEvalTime;
    Timing::Tick lastRelEvalTime;
    Timing::Tick curSampleTime;
    Timing::Tick lastSampleTime;
    Timing::Tick timeOffset;
    float timeFactor;
    float blendWeight;
    bool isPaused;
};
inline void
AnimJob::SetEnqueueMode(AnimJobEnqueueMode::Code m)
{
    this->enqueueMode = m;
}
inline AnimJobEnqueueMode::Code
AnimJob::GetEnqueueMode() const
{
    return this->enqueueMode;
}
inline void
AnimJob::SetName(const Util::StringAtom& n)
{
    this->name = n;
}
inline const Util::StringAtom&
AnimJob::GetName() const
{
    return this->name;
}
inline void
AnimJob::SetExclusiveTag(IndexT i)
{
    this->exclusiveTag = i;
}
inline IndexT
AnimJob::GetExclusiveTag() const
{
    return this->exclusiveTag;
}
inline void
AnimJob::SetTrackIndex(IndexT i)
{
    this->trackIndex = i;
}
inline IndexT
AnimJob::GetTrackIndex() const
{
    return this->trackIndex;
}
inline void
AnimJob::SetBaseTime(Timing::Tick t)
{
    this->baseTime = t;
}
inline Timing::Tick
AnimJob::GetBaseTime() const
{
    return this->baseTime;
}
inline void
AnimJob::SetStartTime(Timing::Tick t)
{
    this->startTime = t;
    this->lastSampleTime = t;
}
inline Timing::Tick
AnimJob::GetStartTime() const
{
    return this->startTime;
}
inline void
AnimJob::SetDuration(Timing::Tick t)
{
    this->duration = t;
}
inline Timing::Tick
AnimJob::GetDuration() const
{
    return this->duration;
}
inline bool
AnimJob::IsInfinite() const
{
    return (0 == this->duration);
}
inline void
AnimJob::SetFadeInTime(Timing::Tick t)
{
    this->fadeInTime = t;
}
inline Timing::Tick
AnimJob::GetFadeInTime() const
{
    return this->fadeInTime;
}
inline void
AnimJob::SetFadeOutTime(Timing::Tick t)
{
    this->fadeOutTime = t;
}
inline Timing::Tick
AnimJob::GetFadeOutTime() const
{
    return this->fadeOutTime;
}
inline void
AnimJob::SetTime( Timing::Tick time )
{
    this->curSampleTime = this->startTime + time;
    this->lastSampleTime = this->curSampleTime;
}
inline Timing::Tick
AnimJob::GetTime() const
{
    return this->curSampleTime;
}
inline void
AnimJob::SetBlendWeight(float w)
{
    this->blendWeight = w;
}
inline float
AnimJob::GetBlendWeight() const
{
    return this->blendWeight;
}
inline void
AnimJob::SetMask(const Characters::CharacterJointMask* mask)
{
    this->mask = mask;
}
inline const Characters::CharacterJointMask*
AnimJob::GetMask() const
{
    return this->mask;
}
inline void
AnimJob::SetTimeFactor(float f)
{
    this->timeFactor = f;
}
inline float
AnimJob::GetTimeFactor() const
{
    return this->timeFactor;
}
inline void
AnimJob::SetTimeOffset(Timing::Tick t)
{
    this->timeOffset = t;
}
inline Timing::Tick
AnimJob::GetTimeOffset() const
{
    return this->timeOffset;
}
inline bool
AnimJob::IsPaused() const
{
    return this->isPaused;
}
}