//------------------------------------------------------------------------------
//  charactercontext.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "charactercontext.h"
#include "skeletonresource.h"
#include "coreanimation/animationresource.h"
#include "graphics/graphicsserver.h"
#include "visibility/visibilitycontext.h"
#include "models/modelcontext.h"
#include "coreanimation/animsamplemixinfo.h"
#include "coreanimation/animsamplebuffer.h"
#include "models/nodes/characterskinnode.h"
#include "profiling/profiling.h"
#include "resources/resourceserver.h"

using namespace Graphics;
using namespace Resources;
namespace Characters
{

CharacterContext::CharacterContextAllocator CharacterContext::characterContextAllocator;
__ImplementContext(CharacterContext, CharacterContext::characterContextAllocator);

Util::HashTable<Util::StringAtom, CoreAnimation::AnimSampleMask> CharacterContext::masks;
Threading::Event CharacterContext::totalCompletionEvent;
Threading::AtomicCounter CharacterContext::ConstantUpdateCounter = 0;

//------------------------------------------------------------------------------
/**
*/
CharacterContext::CharacterContext()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
CharacterContext::~CharacterContext()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::Create()
{
    __CreateContext();
#ifndef PUBLIC_BUILD
    __bundle.OnRenderDebug = CharacterContext::OnRenderDebug;
#endif
    Graphics::GraphicsServer::Instance()->RegisterGraphicsContext(&__bundle, &__state);
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::Setup(
    const Graphics::GraphicsEntityId id
    , const Resources::ResourceName& skeletonResource
    , const IndexT skeletonIndex
    , const Resources::ResourceName& animationResource
    , const IndexT animationIndex
    , const Util::StringAtom& tag
    , bool supportBlending
)
{
    const ContextEntityId cid = GetContextId(id);
    n_assert_fmt(cid != InvalidContextEntityId, "Объект %d не зарегистрирован в CharacterContext", id.HashCode());
    characterContextAllocator.Set<Loaded>(cid.id, NoneLoaded);
    characterContextAllocator.Set<EntityId>(cid.id, id);

    // проверьте, что мы зарегистрировали этот объект для наблюдения, затем получите контекст видимости
    const ContextEntityId visId = Visibility::ObservableContext::GetContextId(id);
    n_assert_fmt(visId != InvalidContextEntityId, "Сущность %d должна быть настроена как наблюдаемая перед символом!", id.HashCode());

    // получить контекст модели
    const ContextEntityId mdlId = Models::ModelContext::GetContextId(id);
    n_assert_fmt(mdlId != InvalidContextEntityId, "Сущность %d необходимо настроить в качестве модели перед персонажем!", id.HashCode());

    const Models::NodeInstanceRange& nodeRange = Models::ModelContext::GetModelRenderableRange(id);
    const Models::ModelContext::ModelInstance::Renderable& renderables = Models::ModelContext::GetModelRenderables();

    for (SizeT i = nodeRange.begin; i < nodeRange.end; i++)
    {
        if (renderables.nodeTypes[i] == Models::CharacterSkinNodeType)
        {
            // Сохраните смещение в узле символа в модели.
            characterContextAllocator.Set<CharacterSkinNodeIndexOffset>(cid.id, i - nodeRange.begin);
            break;
        }
    }

    characterContextAllocator.Get<Skeleton>(cid.id) = Characters::InvalidSkeletonId;
    Resources::CreateResource(skeletonResource, tag, [cid, skeletonIndex](Resources::ResourceId rid)
        {
            auto skeleton = SkeletonResourceGetSkeleton(rid, skeletonIndex);
            characterContextAllocator.Set<Skeleton>(cid.id, skeleton);
            characterContextAllocator.Get<Loaded>(cid.id) |= SkeletonLoaded;

            const Util::FixedArray<CharacterJoint>& joints = Characters::SkeletonGetJoints(skeleton);
            characterContextAllocator.Get<JobJoints>(cid.id).Resize(joints.Size());

            // настраиваемые соединения, масштабированные соединения и соединения, управляемые пользователем
            characterContextAllocator.Get<JointPalette>(cid.id).Resize(joints.Size());
            characterContextAllocator.Get<JointPaletteScaled>(cid.id).Resize(joints.Size());
            characterContextAllocator.Get<UserControlledJoint>(cid.id).Resize(joints.Size());

            // настройка рабочих швов
            IndexT i;
            for (i = 0; i < joints.Size(); i++)
            {
                const CharacterJoint& joint = joints[i];
                SkeletonJobJoint& jobJoint = characterContextAllocator.Get<JobJoints>(cid.id)[i];
                jobJoint.parentJointIndex = joint.parentJointIndex;
            }

        }, nullptr, true);

    characterContextAllocator.Get<Animation>(cid.id) = CoreAnimation::InvalidAnimationId;
    Resources::CreateResource(animationResource, tag, [cid, animationIndex, supportBlending](Resources::ResourceId rid)
        {
            auto animation = CoreAnimation::AnimationResourceGetAnimation(rid, animationIndex);
            characterContextAllocator.Set<Animation>(cid.id, animation);
            characterContextAllocator.Get<Loaded>(cid.id) |= AnimationLoaded;

            // настроить буфер выборки после завершения загрузки анимации
            characterContextAllocator.Get<SampleBuffer>(cid.id).Setup(animation);
            characterContextAllocator.Set<SupportMix>(cid.id, supportBlending);
        }, nullptr, true);

    // очистить состояние воспроизводимой анимации
    IndexT i;
    for (i = 0; i < MaxNumTracks; i++)
    {
        memset(&characterContextAllocator.Get<TrackController>(cid.id).playingAnimations[i], 0, sizeof(AnimationRuntime));
        characterContextAllocator.Get<TrackController>(cid.id).playingAnimations[i].clip = -1;
    }
}

//------------------------------------------------------------------------------
/**
*/
IndexT 
CharacterContext::GetClipIndex(const Graphics::GraphicsEntityId id, const Util::StringAtom& name)
{
    const ContextEntityId cid = GetContextId(id);
    return CoreAnimation::AnimGetIndex(characterContextAllocator.Get<Animation>(cid.id), name);
}

//------------------------------------------------------------------------------
/**
*/
bool 
CharacterContext::PlayClip(
    const Graphics::GraphicsEntityId id, 
    const CoreAnimation::AnimSampleMask* mask,
    const IndexT clipIndex, 
    const IndexT track, 
    const EnqueueMode mode, 
    const float weight, 
    const SizeT loopCount, 
    const float startTime, 
    const float fadeIn, 
    const float fadeOut, 
    const float timeOffset, 
    const float timeFactor)
{
    n_assert(track < MaxNumTracks);
    const ContextEntityId cid = GetContextId(id);
    const CoreAnimation::AnimClip& clip = CoreAnimation::AnimGetClip(characterContextAllocator.Get<Animation>(cid.id), clipIndex);

    // среда выполнения анимации — это контекст, используемый для отслеживания запуска анимации
    AnimationRuntime runtime;
    runtime.clip = clipIndex;
    runtime.enqueueMode = mode;
    runtime.blend = weight;
    runtime.fadeInTime = fadeIn;
    runtime.fadeOutTime = fadeOut;
    runtime.timeFactor = timeFactor;
    runtime.timeOffset = timeOffset;
    runtime.startTime = startTime;
    runtime.curveSampleIndices.Resize(clip.numCurves);
    runtime.curveSampleIndices.Fill(0, clip.numCurves, 0x0);
    runtime.mask = mask;

    runtime.baseTime = 0;
    runtime.duration = 0;
    runtime.evalTime = runtime.prevEvalTime = 0;
    runtime.sampleTime = runtime.prevSampleTime = 0;
    runtime.paused = false;
    runtime.duration = Timing::Tick(clip.duration * loopCount * (1 / timeFactor));

    // добавить к ожидающим анимациям
    if (mode == Replace)
        characterContextAllocator.Get<TrackController>(cid.id).playingAnimations[track] = std::move(runtime);
    else if (mode == Append)
        characterContextAllocator.Get<TrackController>(cid.id).pendingAnimations[track].Append(std::move(runtime));
    else if (mode == IgnoreIfSame)
    {
        const AnimationRuntime& current = characterContextAllocator.Get<TrackController>(cid.id).playingAnimations[track];

        // if clip index doesn't match, append animation
        if (current.clip != clipIndex)
            characterContextAllocator.Get<TrackController>(cid.id).pendingAnimations[track].Append(std::move(runtime));
    }

    return false;
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::StopTrack(const Graphics::GraphicsEntityId id, const IndexT track)
{
    n_assert(track < MaxNumTracks);
    const ContextEntityId cid = GetContextId(id);
    AnimationTracks& tracks = characterContextAllocator.Get<TrackController>(cid.id);

    tracks.pendingAnimations[track].Clear();
    tracks.playingAnimations[track].clip = -1;
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::StopAllTracks(const Graphics::GraphicsEntityId id)
{
    const ContextEntityId cid = GetContextId(id);
    AnimationTracks& tracks = characterContextAllocator.Get<TrackController>(cid.id);

    // просто очистите все анимации обработки
    for (IndexT i = 0; i < MaxNumTracks; i++)
    {
        tracks.pendingAnimations[i].Clear();
        tracks.playingAnimations[i].clip = -1;
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::PauseTrack(const Graphics::GraphicsEntityId id, const IndexT track)
{
    n_assert(track < MaxNumTracks);
    const ContextEntityId cid = GetContextId(id);
    AnimationTracks& tracks = characterContextAllocator.Get<TrackController>(cid.id);

    // установлен для всех ожидающих треков и текущего трека
    IndexT i;
    for (i = 0; i < tracks.pendingAnimations[track].Size(); i++)
        tracks.pendingAnimations[track][i].paused = !tracks.pendingAnimations[track][i].paused;

    tracks.playingAnimations[track].paused = !tracks.playingAnimations[track].paused;
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::Seek(const Graphics::GraphicsEntityId id, const float time)
{
    const ContextEntityId cid = GetContextId(id);
    characterContextAllocator.Get<AnimTime>(cid.id) = time;
}

//------------------------------------------------------------------------------
/**
*/
const float 
CharacterContext::GetTime(const Graphics::GraphicsEntityId id)
{
    const ContextEntityId cid = GetContextId(id);
    return characterContextAllocator.Get<AnimTime>(cid.id);
}

//------------------------------------------------------------------------------
/**
*/
bool 
CharacterContext::IsPlaying(const Graphics::GraphicsEntityId id)
{
    const ContextEntityId cid = GetContextId(id);

    // if any track is playing, this entity is playing
    bool ret = false;
    for (IndexT i = 0; i < MaxNumTracks; i++)
        ret |= characterContextAllocator.Get<TrackController>(cid.id).playingAnimations[i].clip != -1;

    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::SetTrackWeight(const Graphics::GraphicsEntityId id, const IndexT track, const float weight)
{
    const ContextEntityId cid = GetContextId(id);
    AnimationTracks& tracks = characterContextAllocator.Get<TrackController>(cid.id);

    // установлен для всех ожидающих треков и текущего трека
    IndexT i;
    for (i = 0; i < tracks.pendingAnimations[track].Size(); i++)
        tracks.pendingAnimations[track][i].blend = weight;

    tracks.playingAnimations[track].blend = weight;
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::SetTrackTimeFactor(const Graphics::GraphicsEntityId id, const IndexT track, const float factor)
{
    const ContextEntityId cid = GetContextId(id);
    AnimationTracks& tracks = characterContextAllocator.Get<TrackController>(cid.id);

    // установлен для всех ожидающих треков и текущего трека
    IndexT i;
    for (i = 0; i < tracks.pendingAnimations[track].Size(); i++)
        tracks.pendingAnimations[track][i].timeFactor = factor;

    tracks.playingAnimations[track].timeFactor = factor;
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::QueryClips(const Graphics::GraphicsEntityId id, Util::FixedArray<CoreAnimation::AnimClip>& outClips)
{
    const ContextEntityId cid = GetContextId(id);
    const CoreAnimation::AnimationId& animRes = characterContextAllocator.Get<Animation>(cid.id);
    outClips = CoreAnimation::AnimGetClips(animRes);
}

//------------------------------------------------------------------------------
/**
*/
const bool
IsExpired(const CharacterContext::AnimationRuntime& runtime, const Timing::Time time)
{
    if (runtime.duration == 0)
        return false;
    else
        return runtime.startTime + runtime.baseTime + runtime.duration <= time;
}

//------------------------------------------------------------------------------
/**
*/
const bool
IsInfinite(const CharacterContext::AnimationRuntime& runtime)
{
    return runtime.duration == 0;
}

//------------------------------------------------------------------------------
/**
*/
Timing::Tick
GetAbsoluteStopTime(const CharacterContext::AnimationRuntime& runtime)
{
    n_assert(!IsInfinite(runtime));
    return (runtime.baseTime + runtime.startTime + runtime.duration) - runtime.fadeOutTime;
}

struct CharacterJobContext
{
    const Util::Array<Timing::Time>* times;
    const Util::Array<CharacterContext::AnimationTracks>* tracks;
    const Util::Array<CoreAnimation::AnimationId>* anims;
    const Util::Array<CoreAnimation::AnimSampleBuffer>* sampleBuffers;
    const Util::Array<Util::FixedArray<SkeletonJobJoint>>* jobJoints;
    const Util::Array<SkeletonId>* skeletons;
    const Util::Array<Util::FixedArray<Math::mat4>>* jointPalettes;
    const Util::Array<Util::FixedArray<Math::mat4>>* scaledJointPalettes;
    const Util::Array<Util::FixedArray<Math::mat4>>* userJoints;
    float** tmpSamples;
    uint** tmpSampleIndices;
    Math::mat4** tmpJoints;
    
    const Util::Array<Graphics::GraphicsEntityId>* entities;
    CoreAnimation::AnimSampleMixInfo* animMixInfos;
    float frameTime;
    Timing::Tick time;
    Timing::Tick ticks;
};

//------------------------------------------------------------------------------
/**
*/
void
EvalCharacter(SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset, void* ctx)
{
    N_SCOPE(EvalCharacter, Graphics);
    auto context = static_cast<CharacterJobContext*>(ctx);
    using namespace CoreAnimation;

    for (IndexT i = 0; i < groupSize; i++)
    {
        IndexT index = invocationOffset + i;
        if (index >= totalJobs)
            return;

        // обновить время, получить контроллер трека
        Timing::Time& currentTime = context->times->Get(index);
        currentTime += context->frameTime;
        CharacterContext::AnimationTracks& trackController = context->tracks->Get(index);
        const AnimationId anim = context->anims->Get(index);
        if (anim == InvalidAnimationId)
            continue;
        const Util::FixedArray<SkeletonJobJoint>& jobJoint = context->jobJoints->Get(index);
        const SkeletonId skeleton = context->skeletons->Get(index);
        if (skeleton == InvalidSkeletonId)
            continue;
        const Util::FixedArray<Math::mat4>& bindPose = Characters::SkeletonGetBindPose(skeleton);
        const Util::FixedArray<Characters::CharacterJoint>& joints = Characters::SkeletonGetJoints(skeleton);
        const Util::FixedArray<Math::mat4>& jointPalette = context->jointPalettes->Get(index);
        const Util::FixedArray<Math::mat4>& scaledJointPalette = context->scaledJointPalettes->Get(index);
        const Util::FixedArray<Math::vec4>& idleSamples = Characters::SkeletonGetIdleSamples(skeleton);
        const CoreAnimation::AnimSampleBuffer& sampleBuffer = context->sampleBuffers->Get(index);
        Math::mat4* tmpMatrices = context->tmpJoints[index];
        float* tmpSamples = context->tmpSamples[index];
        uint* tmpSampleIndices = context->tmpSampleIndices[index];
        auto sampleMixInfo = context->animMixInfos + index;
        bool runSkeletonThisFrame = false;

        // перебирать все треки и обновлять воспроизводимый клип на каждом соответствующем треке
        bool firstAnimTrack = true;
        IndexT j;
        for (j = 0; j < CharacterContext::MaxNumTracks; j++)
        {
            // запустить анимацию на этом треке
            CharacterContext::AnimationRuntime& playing = trackController.playingAnimations[j];
            bool startedNew = false;

            // обновить ожидающие анимации
            IndexT k;
            for (k = 0; k < trackController.pendingAnimations[j].Size(); k++)
            {
                CharacterContext::AnimationRuntime& pending = trackController.pendingAnimations[j][k];
                if (playing.clip == -1)
                {
                    // ни один клип не воспроизводится, это автоматически заменяет
                    pending.baseTime = context->time;
                    startedNew = true;
                    playing = pending;
                }
                else
                {
                    if (pending.enqueueMode == Characters::Replace)
                    {
                        pending.baseTime = GetAbsoluteStopTime(playing);
                        startedNew = true;
                        playing = pending;
                    }
                    else if (pending.enqueueMode == Characters::IgnoreIfSame)
                    {
                        if (pending.clip == playing.clip && !IsExpired(playing, currentTime))
                        {
                            // стереть клип, потому что мы его уже проигрываем
                            trackController.pendingAnimations[j].EraseIndex(k);
                            k--;
                            continue;
                        }
                        else
                        {
                            // в противном случае действуйте как замена
                            pending.baseTime = GetAbsoluteStopTime(playing);
                            startedNew = true;
                            playing = pending;
                        }
                    }
                    else if (pending.enqueueMode == Characters::Append && IsInfinite(playing))
                    {

#if NEBULA_DEBUG
                        n_error("Контекст персонажа: невозможно вставить задание анимации «%s», поскольку дорожка заблокирована бесконечным клипом\n", playing.name.AsCharPtr());
#endif
                    }
                    // if none of the above, we have to wait for our current animation to finish
                }

                // if we started a new clip, erase the current pending, and quit the loop
                if (startedNew)
                {
                    trackController.pendingAnimations[j].EraseIndex(k);
                    break;
                }
            }

            // if expired, invalidate job
            if (IsExpired(playing, currentTime))
                trackController.playingAnimations[j].clip = -1;

            // if current playing job on this track is invalid, copy a pending job over
            if (playing.clip == -1)
            {
                if (trackController.pendingAnimations[j].Size() > 0)
                {
                    playing = trackController.pendingAnimations[j].Back();
                    trackController.pendingAnimations[j].EraseBack();
                }
            }

            // это может измениться по сравнению с предыдущим условием, поэтому проверьте его еще раз
            if (playing.clip != -1)
            {
                // время обновления
                playing.evalTime = context->ticks - (playing.baseTime + playing.startTime);
                playing.prevEvalTime = playing.evalTime;
                playing.sampleTime = playing.startTime + context->ticks;
                playing.prevSampleTime = playing.sampleTime;

                // if not paused, update sample time
                if (!playing.paused)
                {
                    int timeDivider = Math::frnd(1 / playing.timeFactor);
                    Timing::Tick frameTicks = playing.evalTime - playing.prevEvalTime;
                    Timing::Tick timeDiff = frameTicks / timeDivider;
                    playing.sampleTime = playing.prevSampleTime + timeDiff;
                    playing.prevSampleTime = playing.sampleTime;
                    playing.sampleTime += playing.timeOffset;
                }

                // Необходимо вычислить вес выборки и указатели на ключи «до» и «после».
                const CoreAnimation::AnimClip& clip = CoreAnimation::AnimGetClip(anim, playing.clip);
                const Ptr<AnimKeyBuffer>& buffer = AnimGetBuffer(anim);

                // Создать рабочую память
                Memory::Clear(sampleMixInfo, sizeof(AnimSampleMixInfo));
                sampleMixInfo->sampleType = SampleType::Linear;
                sampleMixInfo->velocityScale.set(playing.timeFactor, playing.timeFactor, playing.timeFactor, 0);

                // Получить указатели на память и размер
                const float* srcPtr = buffer->GetKeyBufferPointer();
                const AnimKeyBuffer::Interval* srcTimePtr = buffer->GetIntervalBufferPointer();

                const Util::FixedArray<AnimCurve>& curves = CoreAnimation::AnimGetCurves(anim);
                Timing::Tick evalTime = playing.sampleTime % clip.duration;

                if (firstAnimTrack
                    || playing.blend != 1.0f)
                {
                    if (sampleMixInfo->sampleType == SampleType::Step)
                        AnimSampleStep(clip, curves, evalTime, sampleMixInfo->velocityScale, idleSamples, srcPtr, srcTimePtr, playing.curveSampleIndices.Begin(), sampleBuffer.GetSamplesPointer(), sampleBuffer.GetSampleCountsPointer());
                    else
                        AnimSampleLinear(clip, curves, evalTime, sampleMixInfo->velocityScale, idleSamples, srcPtr, srcTimePtr, playing.curveSampleIndices.Begin(), sampleBuffer.GetSamplesPointer(), sampleBuffer.GetSampleCountsPointer());
                }
                else // Играем с миксом
                {
                    uchar tmpSampleCounts = 0;
                    if (sampleMixInfo->sampleType == SampleType::Step)
                        AnimSampleStep(clip, curves, evalTime, sampleMixInfo->velocityScale, idleSamples, srcPtr, srcTimePtr, tmpSampleIndices, tmpSamples, &tmpSampleCounts);
                    else
                        AnimSampleLinear(clip, curves, evalTime, sampleMixInfo->velocityScale, idleSamples, srcPtr, srcTimePtr, tmpSampleIndices, tmpSamples, &tmpSampleCounts);

                    AnimMix(clip, curves.Size(), playing.mask, sampleMixInfo->mixWeight, sampleBuffer.GetSamplesPointer(), tmpSamples, sampleBuffer.GetSampleCountsPointer(), &tmpSampleCounts, sampleBuffer.GetSamplesPointer(), sampleBuffer.GetSampleCountsPointer());
                }

                // Запускаем скелет, как только в очереди появится одно задание анимации.
                runSkeletonThisFrame = true;

                // переверните первый флаг дорожки анимации, что запустит следующее задание для микширования
                firstAnimTrack = false;
            }

        }

        // Если у нас нет анимации, просто пропустите обновление скелета персонажа.
        if (!runSkeletonThisFrame)
            continue;

        // Оценить скелет
        const Math::mat4* invPoseMatrixBase = bindPose.Begin();
        Math::mat4* unscaledMatrixBase = tmpMatrices;

        Math::vec3 translate(0.0f, 0.0f, 0.0f);
        Math::vec3 scale(1.0f, 1.0f, 1.0f);
        Math::quat rotate;

        // загрузить указатели из контекста
        // ПРИМЕЧАНИЕ. Указатель sampleBase может иметь значение NULL, если нет допустимой анимации.
        // данные существуют, в этом случае скелет должен быть просто установлен
        // в позу Иисуса
        const SkeletonJobJoint* compsBase = jobJoint.Begin();
        n_assert(0 != compsBase);
        const float* samplesBase = sampleBuffer.GetSamplesPointer();
        const float* samplesPtr = samplesBase;

        Math::mat4* scaledMatrixBase = scaledJointPalette.Begin();
        Math::mat4* skinMatrixBase = jointPalette.Begin();

        // входные сэмплы могут дополнительно включать сэмплы скорости, которые нам нужно пропустить...
        uint sampleWidth = (sampleBuffer.GetNumSamples() / jointPalette.Size());

        // вычислить количество суставов
        int jointIndex;
        for (jointIndex = 0; jointIndex < jointPalette.Size(); jointIndex++)
        {
            static const size_t TRANSLATION_OFFSET = 0;
            static const size_t ROTATION_OFFSET = 3;
            static const size_t SCALE_OFFSET = 7;

            translate.load(samplesPtr + TRANSLATION_OFFSET);
            rotate.load(samplesPtr + ROTATION_OFFSET);
            scale.load(samplesPtr + SCALE_OFFSET);

            samplesPtr += sampleWidth;

            const SkeletonJobJoint& comps = compsBase[jointIndex];

            Math::mat4& unscaledMatrix = unscaledMatrixBase[jointIndex];
            Math::mat4& scaledMatrix = scaledMatrixBase[jointIndex];

            Math::vec3 baseScale = xyz(idleSamples[jointIndex * 3 + 2]);

            // обновить немасштабированную матрицу
            // вращение анимации
            scaledMatrix = Math::affine(scale, rotate, translate);
            unscaledMatrix = Math::affine(Math::vec3(1), rotate, translate);
            if (InvalidIndex != comps.parentJointIndex)
            {
                const Math::mat4& parentUnscaledMatrix = unscaledMatrixBase[comps.parentJointIndex];
                scaledMatrix = parentUnscaledMatrix * scaledMatrix;
                unscaledMatrix = parentUnscaledMatrix * unscaledMatrix;
            }

            skinMatrixBase[jointIndex] = scaledMatrix * invPoseMatrixBase[jointIndex];
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::UpdateAnimations(const Graphics::FrameContext& ctx)
{
    N_SCOPE(CharacterBeforeFrame, Character);
    using namespace CoreAnimation;
    const Util::Array<Timing::Time>& times = characterContextAllocator.GetArray<AnimTime>();
    const Util::Array<AnimationTracks>& tracks = characterContextAllocator.GetArray<TrackController>();
    const Util::Array<AnimationId>& anims = characterContextAllocator.GetArray<Animation>();
    const Util::Array<CoreAnimation::AnimSampleBuffer>& sampleBuffers = characterContextAllocator.GetArray<SampleBuffer>();
    const Util::Array<Util::FixedArray<SkeletonJobJoint>>& jobJoints = characterContextAllocator.GetArray<JobJoints>();
    const Util::Array<SkeletonId>& skeletons = characterContextAllocator.GetArray<Skeleton>();
    const Util::Array<Util::FixedArray<Math::mat4>>& jointPalettes = characterContextAllocator.GetArray<JointPalette>();
    const Util::Array<Util::FixedArray<Math::mat4>>& scaledJointPalettes = characterContextAllocator.GetArray<JointPaletteScaled>();
    const Util::Array<Util::FixedArray<Math::mat4>>& userJoints = characterContextAllocator.GetArray<UserControlledJoint>();
    const Util::Array<Graphics::GraphicsEntityId>& models = characterContextAllocator.GetArray<EntityId>();
    const Util::Array<bool>& supportsBlending = characterContextAllocator.GetArray<SupportMix>();
    const Util::Array<IndexT>& characterSkinNodeIndices = characterContextAllocator.GetArray<CharacterSkinNodeIndexOffset>();

    if (!models.IsEmpty())
    {
        static Threading::AtomicCounter animationCounter = 0;

        // Установить общий счетчик
        n_assert(animationCounter == 0);
        animationCounter = 1;

        CharacterJobContext charCtx;
        charCtx.times = &times;
        charCtx.tracks = &tracks;
        charCtx.anims = &anims;
        charCtx.sampleBuffers = &sampleBuffers;
        charCtx.jobJoints = &jobJoints;
        charCtx.skeletons = &skeletons;
        charCtx.jointPalettes = &jointPalettes;
        charCtx.scaledJointPalettes = &scaledJointPalettes;
        charCtx.userJoints = &userJoints;
        charCtx.entities = &models;
        charCtx.frameTime = ctx.frameTime;
        charCtx.ticks = ctx.ticks;
        charCtx.time = ctx.time;
        charCtx.animMixInfos = Jobs2::JobAlloc<AnimSampleMixInfo>(models.Size());

        charCtx.tmpJoints = Jobs2::JobAlloc<Math::mat4*>(models.Size());
        charCtx.tmpSampleIndices = Jobs2::JobAlloc<uint*>(models.Size());
        charCtx.tmpSamples = Jobs2::JobAlloc<float*>(models.Size());

        IndexT i;
        for (i = 0; i < models.Size(); i++)
        {
            if (models[i] == Graphics::InvalidGraphicsEntityId)
                continue;

            // Выделить рабочую память для преобразований символов
            const Util::FixedArray<Math::mat4>& jointPalette = jointPalettes[i];
            charCtx.tmpJoints[i] = Jobs2::JobAlloc<Math::mat4>(jointPalette.Size());

            // Выделение рабочей памяти для микширования анимации.
            const CoreAnimation::AnimSampleBuffer& sampleBuffer = sampleBuffers[i];
            if (supportsBlending[i])
            {
                charCtx.tmpSampleIndices[i] = Jobs2::JobAlloc<uint>(jointPalette.Size() * 3);
                charCtx.tmpSamples[i] = Jobs2::JobAlloc<float>(sampleBuffer.GetNumSamples());
            }
        }

        // Запустить задание
        Jobs2::JobDispatch(EvalCharacter, models.Size(), 64, charCtx, nullptr, &animationCounter, nullptr);

        n_assert(ConstantUpdateCounter == 0);
        ConstantUpdateCounter = 1;

        // Запустить задание для обновления констант
        Jobs2::JobDispatch(
            [
                characterNodeIndices = characterSkinNodeIndices.ConstBegin()
                , entities = models.ConstBegin()
                , jointPalettes = jointPalettes.ConstBegin()
            ]
        (SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
        {
            N_SCOPE(JointConstantsUpdate, Graphics);
            for (IndexT i = 0; i < groupSize; i++)
            {
                IndexT index = invocationOffset + i;
                if (index >= totalJobs)
                    return;

                const Graphics::GraphicsEntityId entity = entities[index];
                if (entity == Graphics::InvalidGraphicsEntityId)
                    continue;

                const Models::NodeInstanceRange& range = Models::ModelContext::GetModelRenderableRange(entity);
                const Models::ModelContext::ModelInstance::Renderable& renderables = Models::ModelContext::GetModelRenderables();

                const Util::FixedArray<Math::mat4>& jointPalette = jointPalettes[index];
                IndexT node = range.begin + characterNodeIndices[index];
                n_assert(renderables.nodeTypes[node] == Models::NodeType::CharacterSkinNodeType);
                Models::CharacterSkinNode* sparent = reinterpret_cast<Models::CharacterSkinNode*>(renderables.nodes[node]);
                const Util::Array<IndexT>& usedIndices = sparent->skinFragments[0].jointPalette;
                Util::FixedArray<Math::mat4, true> usedMatrices(usedIndices.Size());

                // обновить суставы, которые хранятся в контексте символа
                if (!jointPalette.IsEmpty())
                {
                    // скопируйте палитру активной матрицы или установите идентификатор
                    IndexT j;
                    for (j = 0; j < usedIndices.Size(); j++)
                    {
                        const IndexT lookup = usedIndices[j];
                        usedMatrices[j] = jointPalette[lookup];
                    }
                }

                // Обновить палитру скинов
                uint64_t offset = CoreGraphics::SetConstants(usedMatrices.Begin(), usedMatrices.Size());
                renderables.nodeStates[node].resourceTableOffsets[renderables.nodeStates[node].skinningConstantsIndex] = offset;
            }

        }, characterSkinNodeIndices.Size(), 64, { &animationCounter }, &ConstantUpdateCounter, &CharacterContext::totalCompletionEvent);
    }
    else // Если у нас нет вакансий, просто сообщите о событии завершения
        CharacterContext::totalCompletionEvent.Signal();
}

//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::WaitForCharacterJobs(const Graphics::FrameContext& ctx)
{
    N_MARKER_BEGIN(WaitForCharacter, Graphics);
    CharacterContext::totalCompletionEvent.Wait();
    N_MARKER_END();
}

//------------------------------------------------------------------------------
/**
*/
CoreAnimation::AnimSampleMask* 
CharacterContext::CreateAnimSampleMask(const Util::StringAtom& name, const Util::FixedArray<Math::scalar>& weights)
{
    IndexT index = CharacterContext::masks.Add(name, CoreAnimation::AnimSampleMask{ name, weights });
    return &CharacterContext::masks.ValueAtIndex(name, index);
}

//------------------------------------------------------------------------------
/**
*/
CoreAnimation::AnimSampleMask*
CharacterContext::GetAnimSampleMask(const Util::StringAtom& name)
{
    IndexT index = CharacterContext::masks.FindIndex(name);
    n_assert(index != InvalidIndex);
    return &CharacterContext::masks.ValueAtIndex(name, index);
}

#ifndef PUBLIC_BUILD 
//------------------------------------------------------------------------------
/**
*/
void 
CharacterContext::OnRenderDebug(uint32_t flags)
{
    const Util::Array<Util::FixedArray<Math::mat4>>& jointPalettes = characterContextAllocator.GetArray<JointPaletteScaled>();
    const Util::Array<Graphics::GraphicsEntityId>& modelContexts = characterContextAllocator.GetArray<EntityId>();
    const Math::mat4 scale = Math::scaling(0.1f, 0.1f, 0.1f);
    IndexT i;
    for (i = 0; i < jointPalettes.Size(); i++)
    {
        if (modelContexts[i] == Graphics::InvalidGraphicsEntityId)
            continue;

        const Util::FixedArray<Math::mat4>& jointsPalette = jointPalettes[i];
        const Math::mat4& transform = Models::ModelContext::GetTransform(modelContexts[i]);
        IndexT j;
        for (j = 0; j < jointsPalette.Size(); j++)
        {
            Math::mat4 joint = transform * jointsPalette[j];
            Math::mat4 mat;
            mat.row0 = normalize3(joint.row0);
            mat.row1 = normalize3(joint.row1);
            mat.row2 = normalize3(joint.row2);
            mat.row3 = normalize3(joint.row3);
            mat.row3.w = 1.0f;
            mat.position = joint.position;
            mat = mat * scale;
            CoreGraphics::RenderShape shape;
            shape.SetupSimpleShape(CoreGraphics::RenderShape::Sphere, CoreGraphics::RenderShape::RenderFlag(CoreGraphics::RenderShape::AlwaysOnTop), Math::vec4(1, 0, 0, 0.5f), mat);
            CoreGraphics::ShapeRenderer::Instance()->AddShape(shape);
            //Im3d::Im3dContext::DrawSphere(joint, Math::vec4(1,0,0,0.5f));
        }
    }
}
    
#endif

} // Символы пространства имен

