//------------------------------------------------------------------------------
//  playclipjob.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "animation/playclipjob.h"
#include "animation/animsequencer.h"
#include "coreanimation/animeventemitter.h"
#include "coreanimation/animutil.h"
#include "jobs/job.h"

namespace Animation
{
__ImplementClass(Animation::PlayClipJob, 'PCLJ', Animation::AnimJob);

using namespace CoreAnimation;
using namespace Jobs;

//------------------------------------------------------------------------------
/**
*/
PlayClipJob::PlayClipJob() :
    clipIndex(InvalidIndex),
    loopCount(1.0f),
    firstAnimEventCheck(true)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
PlayClipJob::~PlayClipJob()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void 
PlayClipJob::OnAttachedToSequencer(const AnimSequencer& animSequencer)
{
    n_assert(this->clipName.IsValid());

    // поиск индекса и продолжительности клипа
    const Ptr<AnimResource>& animRes = animSequencer.GetAnimResource();
    this->name = this->clipName;
    this->clipIndex = animRes->GetClipIndexByName(this->clipName);
    if (InvalidIndex == this->clipIndex)
    {
        n_error("PlayClipJob: анимационный клип «%s» не существует в «%s»!\n",
            this->clipName.Value(), animRes->GetResourceId().Value());
    }
    Timing::Tick clipDuration = animRes->GetClipByIndex(this->clipIndex).GetClipDuration();

    // переопределить продолжительность воспроизведения из количества циклов
    this->SetDuration(Timing::Tick(clipDuration * this->loopCount * (1 / this->timeFactor)));

    // установить флаг первого кадра для событий анимации
    this->firstAnimEventCheck = true;

    // наконец вызовите родительский класс
    AnimJob::OnAttachedToSequencer(animSequencer);
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Job>
PlayClipJob::CreateEvaluationJob(Timing::Tick time, const Ptr<AnimSampleBuffer>& optMixIn, const Ptr<AnimSampleBuffer>& resultBuffer)
{
    n_assert(this->IsActive(time));
    n_assert(this->IsAttachedToSequencer());
    n_assert(InvalidIndex != this->clipIndex);
    Ptr<Job> job;

    // проверьте, необходимо ли выполнять смешивание
    if (!optMixIn.isvalid())
    {
        // без смешивания
        job = AnimUtil::CreateSampleJob(this->animSequencer->GetAnimResource(),
                                        this->clipIndex,
                                        SampleType::Linear,
                                        this->curSampleTime,
                                        this->timeFactor,
                                        this->mask,
                                        resultBuffer);
    }
    else
    {
        // выполнять как отбор проб, так и микширование
        float curMixWeight = this->ComputeBlendWeight(this->curRelEvalTime);
        job = AnimUtil::CreateSampleAndMixJob(this->animSequencer->GetAnimResource(),
                                              this->clipIndex,
                                              SampleType::Linear,
                                              this->curSampleTime,
                                              this->timeFactor,
                                              this->mask,
                                              curMixWeight,
                                              optMixIn,
                                              resultBuffer);
    }
    return job;
}

//------------------------------------------------------------------------------
/**
    Собирает анимевенты, активные в определенном временном диапазоне, и заполняет массив информации анимевентов.

    ФИКСМЕ ФИКСМ ФИКСМ
    ЭТО ВЫГЛЯДИТ СЛИШКОМ СЛОЖНО!
*/
Util::Array<AnimEventInfo> 
PlayClipJob::EmitAnimEvents(Timing::Tick startTimeEvents, Timing::Tick endTimeEvents, const Util::String& optionalCatgeory)
{
    Util::Array<AnimEventInfo> eventInfos;
    n_assert(this->animSequencer != 0);
    const AnimClip& clip = this->animSequencer->GetAnimResource()->GetClipByIndex(this->clipIndex);

    int timeDivider = Math::n_frnd(1 / this->timeFactor);

    // отобразить абсолютное время для обрезки относительного времени
    Timing::Tick nettoClipDuration = clip.GetClipDuration();
    Timing::Tick relStartTime = startTimeEvents - this->baseTime;
    Timing::Tick duration = endTimeEvents - startTimeEvents;
    Timing::Tick relEndTime = relStartTime + (Timing::Tick)(float(duration));
    relStartTime /= timeDivider;
    relEndTime /= timeDivider;

    // if we have a cycle we also need to cycle the events
    if (clip.GetPostInfinityType() == InfinityType::Cycle)
    {
        relStartTime %= nettoClipDuration;
        relEndTime %= nettoClipDuration;
    }

    if (this->firstAnimEventCheck)
    {
        // if its the first time we check for animevents, (its done on gfxthread)
        // вычесть время последней оценки
        relStartTime = Math::n_iclamp(relStartTime - (this->lastSampleTime - this->timeOffset), 0, relStartTime);        
        this->firstAnimEventCheck = false;
    }

    Util::Array<AnimEvent> events = AnimEventEmitter::EmitAnimEvents(clip, relStartTime, relEndTime, this->IsInfinite());
    //n_printf("PlayClipJob::EmitAnimEvents: число событий %i\n",events.Size());
    IndexT i;
    for (i = 0; i < events.Size(); ++i)
    {
        if (optionalCatgeory.IsEmpty() || 
            events[i].GetCategory().IsValid() && 
            optionalCatgeory == events[i].GetCategory().Value())
        {
            AnimEventInfo newInfo;
            newInfo.SetAnimEvent(events[i]);
            newInfo.SetAnimJobName(this->GetName());
            newInfo.SetWeight(this->blendWeight);
            eventInfos.Append(newInfo);
        }
    }
    return eventInfos;
}

} // пространство имен Анимация
