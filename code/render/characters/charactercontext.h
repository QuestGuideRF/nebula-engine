#pragma once
#include "graphics/graphicscontext.h"
#include "resources/resourceid.h"
#include "coreanimation/animclip.h"
#include "coreanimation/animsamplemask.h"
#include "characters/skeleton.h"
#include "coreanimation/animation.h"
#include "coreanimation/animsamplebuffer.h"
#include "characters/skeletonjoint.h"
#include "jobs/jobs.h"
namespace CoreAnimation
{
extern void AnimSampleJob(SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset, void* ctx);
extern void AnimSampleJobWithMix(SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset, void* ctx);
}
namespace Characters
{
enum EnqueueMode
{
    Append,
    Replace,
    IgnoreIfSame
};
class CharacterContext : public Graphics::GraphicsContext
{
    __DeclareContext();
public:
    CharacterContext();
    virtual ~CharacterContext();
    static void Create();
    static void Setup(
        const Graphics::GraphicsEntityId id
        , const Resources::ResourceName& skeletonResource
        , const IndexT skeletonIndex
        , const Resources::ResourceName& animationResource
        , const IndexT animationIndex
        , const Util::StringAtom& tag
        , bool supportBlending = true
    );
    static IndexT GetClipIndex(const Graphics::GraphicsEntityId id, const Util::StringAtom& name);
    static bool PlayClip(
        const Graphics::GraphicsEntityId id,
        const CoreAnimation::AnimSampleMask* mask,
        const IndexT clipIndex,
        const IndexT track,
        const EnqueueMode mode,
        const float weight = 1.0f,
        const SizeT loopCount = 1,
        const float startTime = 0.0f,
        const float fadeIn = 0.0f,
        const float fadeOut = 0.0f,
        const float timeOffset = 0.0f,
        const float timeFactor = 1.0f);
    static void StopTrack(const Graphics::GraphicsEntityId id, const IndexT track);
    static void StopAllTracks(const Graphics::GraphicsEntityId id);
    static void PauseTrack(const Graphics::GraphicsEntityId id, const IndexT track);
    static void Seek(const Graphics::GraphicsEntityId id, const float time);
    static const float GetTime(const Graphics::GraphicsEntityId id);
    static bool IsPlaying(const Graphics::GraphicsEntityId id);
    static void SetTrackWeight(const Graphics::GraphicsEntityId id, const IndexT track, const float weight);
    static void SetTrackTimeFactor(const Graphics::GraphicsEntityId id, const IndexT track, const float factor);
    static void QueryClips(const Graphics::GraphicsEntityId id, Util::FixedArray<CoreAnimation::AnimClip>& outClips);
    static void UpdateAnimations(const Graphics::FrameContext& ctx);
    static void WaitForCharacterJobs(const Graphics::FrameContext& ctx);
    static CoreAnimation::AnimSampleMask* CreateAnimSampleMask(const Util::StringAtom& name, const Util::FixedArray<Math::scalar>& weights);
    static CoreAnimation::AnimSampleMask* GetAnimSampleMask(const Util::StringAtom& name);
#ifndef PUBLIC_DEBUG
    static void OnRenderDebug(uint32_t flags);
#endif
    enum LoadState
    {
        NoneLoaded = 0,
        SkeletonLoaded = N_BIT(1),
        AnimationLoaded = N_BIT(2),
    };
    static Threading::AtomicCounter ConstantUpdateCounter;
private:
    friend struct CharacterJobContext;
    struct AnimationRuntime
    {
        IndexT clip;
        EnqueueMode enqueueMode;
        Timing::Tick baseTime, startTime;
        Timing::Tick duration;
        Timing::Tick fadeInTime, fadeOutTime;
        Timing::Tick evalTime, prevEvalTime;
        Timing::Tick sampleTime, prevSampleTime;
        Timing::Tick timeOffset;
        Util::Array<uint> curveSampleIndices;
        const CoreAnimation::AnimSampleMask* mask;
        float timeFactor;
        float blend;
        bool paused;
#if NEBULA_DEBUG
        Util::String name;
#endif
    };
    friend const bool IsExpired(const CharacterContext::AnimationRuntime& runtime, const Timing::Time time);
    friend const bool IsInfinite(const CharacterContext::AnimationRuntime& runtime);
    friend Timing::Tick GetAbsoluteStopTime(const CharacterContext::AnimationRuntime& runtime);
    friend void EvalCharacter(SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset, void* ctx);
    static const SizeT MaxNumTracks = 16;
    struct AnimationTracks
    {
        Util::StackArray<AnimationRuntime, 8>   pendingAnimations[MaxNumTracks];
        AnimationRuntime                        playingAnimations[MaxNumTracks];
    };
    enum
    {
        Skeleton,
        Animation,
        Loaded,
        TrackController,
        AnimTime,
        JointPalette,
        JointPaletteScaled,
        UserControlledJoint,
        JobJoints,
        SampleBuffer,
        SupportMix,
        EntityId,
        CharacterSkinNodeIndexOffset
    };
    typedef Ids::IdAllocator<
        SkeletonId,
        CoreAnimation::AnimationId,
        LoadState,
        AnimationTracks,
        Timing::Time,
        Util::FixedArray<Math::mat4>,
        Util::FixedArray<Math::mat4>,
        Util::FixedArray<Math::mat4>,
        Util::FixedArray<SkeletonJobJoint>,
        CoreAnimation::AnimSampleBuffer,
        bool,
        Graphics::GraphicsEntityId,
        IndexT
    > CharacterContextAllocator;
    static CharacterContextAllocator characterContextAllocator;
    static Graphics::ContextEntityId Alloc();
    static void Dealloc(Graphics::ContextEntityId id);
    static Util::HashTable<Util::StringAtom, CoreAnimation::AnimSampleMask> masks;
    static Threading::Event totalCompletionEvent;
};
__ImplementEnumBitOperators(CharacterContext::LoadState);
inline Graphics::ContextEntityId
CharacterContext::Alloc()
{
    return characterContextAllocator.Alloc();
}
inline void
CharacterContext::Dealloc(Graphics::ContextEntityId id)
{
    characterContextAllocator.Dealloc(id.id);
}
}