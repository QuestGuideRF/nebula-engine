#pragma once
#include "animation/animjob.h"
namespace Animation
{
class PlayClipJob : public AnimJob
{
    __DeclareClass(PlayClipJob);
public:
    PlayClipJob();
    virtual ~PlayClipJob();
    void SetClipName(const Util::StringAtom& clipName);
    const Util::StringAtom& GetClipName() const;
    void SetLoopCount(float loopCount);
    float GetLoopCount() const;
private:
    virtual void OnAttachedToSequencer(const AnimSequencer& animSequencer);
    virtual Ptr<Jobs::Job> CreateEvaluationJob(Timing::Tick time, const Ptr<CoreAnimation::AnimSampleBuffer>& mixIn, const Ptr<CoreAnimation::AnimSampleBuffer>& result);
    virtual Util::Array<AnimEventInfo> EmitAnimEvents(Timing::Tick startTime, Timing::Tick endTime, const Util::String& optionalCatgeory);
    Util::StringAtom clipName;
    IndexT clipIndex;
    float loopCount;
    bool firstAnimEventCheck;
};
inline void
PlayClipJob::SetClipName(const Util::StringAtom& n)
{
    this->clipName = n;
}
inline const Util::StringAtom&
PlayClipJob::GetClipName() const
{
    return this->clipName;
}
inline void
PlayClipJob::SetLoopCount(float c)
{
    this->loopCount = c;
}
inline float
PlayClipJob::GetLoopCount() const
{
    return this->loopCount;
}
}