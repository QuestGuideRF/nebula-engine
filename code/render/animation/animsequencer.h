#pragma once
#include "core/refcounted.h"
#include "animation/animjob.h"
#include "animation/animeventinfo.h"
#include "jobs/jobport.h"
#include "coregraphics/rendershape.h"
namespace Animation
{
class AnimSequencer
{
public:
    AnimSequencer();
    ~AnimSequencer();
    void Setup(const Ptr<CoreAnimation::AnimResource>& animResource);
    void Discard();
    bool IsValid() const;
    void SetDebugHudEnabled(bool b);
    bool IsDebugHudEnabled() const;
    void EnqueueAnimJob(const Ptr<AnimJob>& animJob);
    void StopTrack(IndexT trackIndex, bool allowFadeOut=true);
    void StopAllTracks(bool allowFadeOut=true);
    void PauseTrack(IndexT trackIndex, bool pause);
    void PauseAllTracks(bool pause);
    void SetTime(Timing::Tick time);
    void UpdateTime(Timing::Tick time);
    bool StartAsyncEvaluation(const Ptr<Jobs::JobPort>& jobPort);
    Timing::Tick GetTime() const;
    const Ptr<CoreAnimation::AnimSampleBuffer>& GetResult() const;
    const Ptr<CoreAnimation::AnimResource>& GetAnimResource() const;
    Util::Array<Ptr<AnimJob> > GetAllAnimJobs() const;
    Util::Array<Ptr<AnimJob> > GetAnimJobsByTrackIndex(IndexT trackIndex) const;
    Util::Array<Ptr<AnimJob> > GetAnimJobsByName(const Util::StringAtom& name) const;
    Util::Array<AnimEventInfo> EmitAnimEvents(Timing::Tick startTime, Timing::Tick endTime, bool justDominatingJob, const Util::String& optionalCategory = "") const;
private:
    void EnqueueAnimJobForStopping(const Ptr<AnimJob>& animJob);
    void DiscardAnimJob(Ptr<AnimJob> animJob);
    void InsertEnqueuedAnimJobs(Timing::Tick time);
    void UpdateStoppedAnimJobs(Timing::Tick time);
    void RemoveExpiredAnimJobs(Timing::Tick time);
    void UpdateTimeActiveAnimJobs(Timing::Tick time);
    IndexT FindDominatingAnimJobIndex(Timing::Tick startTime, Timing::Tick endTime) const;
    #if NEBULA_ANIMATIONSYSTEM_FRAMEDUMP
    void DumpFrameDebugInfo(Timing::Tick time);
    #endif
    void DebugRenderHud(Timing::Tick time);
    void DebugGenAnimJobRectangle(Timing::Tick timeOrigin, const Ptr<AnimJob>& animJob, Util::Array<CoreGraphics::RenderShape::RenderShapeVertex>& outVertices);
    CoreGraphics::RenderShape::RenderShapeVertex DebugViewSpaceCoord(Timing::Tick timeOrigin, Timing::Tick time, IndexT trackIndex);
    void DebugRenderLineList(const Util::Array<CoreGraphics::RenderShape::RenderShapeVertex>& vertices, const Math::float4& color);
    void DebugRenderAnimJobText(Timing::Tick timeOrigin, const Ptr<AnimJob>& animJob, const Math::float4& color);
    void DebugGenTrackLines(SizeT maxTracks, Util::Array<CoreGraphics::RenderShape::RenderShapeVertex>& outVertices);
    Timing::Tick time;
    Ptr<CoreAnimation::AnimResource> animResource;
    Ptr<CoreAnimation::AnimSampleBuffer> dstSampleBuffer;
    Util::Array<Ptr<AnimJob> > enqueuedAnimJobs;
    Util::Array<Ptr<AnimJob> > stoppedAnimJobs;
    Util::Array<Ptr<AnimJob> > animJobs;
    Util::Array<Ptr<Jobs::Job> > jobChain;
    bool debugHudEnabled;
};
inline Timing::Tick
AnimSequencer::GetTime() const
{
    return this->time;
}
inline bool
AnimSequencer::IsValid() const
{
    return this->animResource.isvalid();
}
inline const Ptr<CoreAnimation::AnimSampleBuffer>&
AnimSequencer::GetResult() const
{
    return this->dstSampleBuffer;
}
inline const Ptr<CoreAnimation::AnimResource>&
AnimSequencer::GetAnimResource() const
{
    return this->animResource;
}
inline void
AnimSequencer::SetDebugHudEnabled(bool b)
{
    this->debugHudEnabled = b;
}
inline bool
AnimSequencer::IsDebugHudEnabled() const
{
    return this->debugHudEnabled;
}
}