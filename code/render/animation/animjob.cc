//------------------------------------------------------------------------------
//  animjob.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "animation/animjob.h"
#include "animation/animsequencer.h"

namespace Animation
{
__ImplementClass(Animation::AnimJob, 'AJOB', Core::RefCounted);

using namespace Jobs;
using namespace Util;
using namespace CoreAnimation;

//------------------------------------------------------------------------------
/**
*/
AnimJob::AnimJob() :
    animSequencer(0),
    mask(0),
    enqueueMode(AnimJobEnqueueMode::Intercept),
    trackIndex(0),
    exclusiveTag(InvalidIndex),
    baseTime(0),
    startTime(0),
    duration(0),
    fadeInTime(0),
    fadeOutTime(0),
    curRelEvalTime(0),
    lastRelEvalTime(0),
    curSampleTime(0),
    lastSampleTime(0),
    timeOffset(0),
    timeFactor(1.0f),
    blendWeight(1.0f),
    isPaused(false)
{
    // empty
}    

//------------------------------------------------------------------------------
/**
*/
AnimJob::~AnimJob()
{
    n_assert(!this->IsAttachedToSequencer());
}

//------------------------------------------------------------------------------
/**
*/
void
AnimJob::OnAttachedToSequencer(const AnimSequencer& animSeq)
{
    n_assert(!this->IsAttachedToSequencer());
    this->animSequencer = &animSeq;

    // if the fade-in and fade-out times add up to be greater then the
    // продолжительность воспроизведения, нам нужно их исправить, чтобы предотвратить проблемы с смешиванием
    this->FixFadeTimes();

    // вычислить продолжительность и время появления/затухания в зависимости от временного фактора
    float timeMultiplier = 1 / float(Math::n_abs(this->timeFactor));
    int timeDivider = Math::n_frnd(1 / float(Math::n_abs(this->timeFactor)));
    Timing::Time durationFactored = Timing::TicksToSeconds(this->duration) / timeDivider;
    Timing::Time fadeInFactored = Timing::TicksToSeconds(this->fadeInTime) / timeDivider;
    Timing::Time fadeOutFactored = Timing::TicksToSeconds(this->fadeOutTime) / timeDivider;

    // конвертировать обратно в тики
    this->duration = Timing::SecondsToTicks(durationFactored);
    this->fadeInTime = Timing::SecondsToTicks(fadeInFactored);
    this->fadeOutTime = Timing::SecondsToTicks(fadeOutFactored);
}

//------------------------------------------------------------------------------
/**
*/
void
AnimJob::OnRemoveFromSequencer()
{
    n_assert(this->IsAttachedToSequencer());
    this->animSequencer = 0;
}

//------------------------------------------------------------------------------
/**
    Этот метод проверяет, будет ли время появления и затухания 
    быть больше продолжительности воспроизведения, если да, это исправит время затухания
    во избежание «всплеска смеси».
*/
void
AnimJob::FixFadeTimes()
{
    if (!this->IsInfinite())
    {
        Timing::Tick fadeTime = this->fadeInTime + this->fadeOutTime;
        if (fadeTime > 0)
        {
            if (fadeTime > this->duration)
            {
                float mul = float(duration) / float(fadeTime);
                this->fadeInTime = Timing::Tick(this->fadeInTime * mul);
                this->fadeOutTime = Timing::Tick(this->fadeOutTime * mul);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
AnimJob::IsAttachedToSequencer() const
{
    return (0 != this->animSequencer);
}

//------------------------------------------------------------------------------
/**
    Этот метод вернет true, если текущее время оценки находится между
    время начала и время окончания задания анимации.
*/
bool
AnimJob::IsActive(Timing::Tick time) const
{
    Timing::Tick absStartTime = this->baseTime + this->startTime;
    Timing::Tick absEndTime = absStartTime + this->duration;
    if (this->IsInfinite())
    {
        return (time >= absStartTime);
    }
    else
    {
        return (time >= absStartTime) && (time < absEndTime);
    }
}

//------------------------------------------------------------------------------
/**
    This method will return true as long as the current eval time is
    before the start time (the job hasn't started yet).
*/
bool
AnimJob::IsPending(Timing::Tick time) const
{
    return (time < (this->baseTime + this->startTime));
}

//------------------------------------------------------------------------------
/**
    Возвращайте true, если задание анимации в настоящее время находится в фазе затухания или после нее.
*/
bool
AnimJob::IsStoppingOrExpired(Timing::Tick time) const
{
    if (this->IsInfinite())
    {
        return false;
    }
    else
    {
        return time >= ((this->baseTime + this->startTime + this->duration) - this->fadeOutTime);
    }
}

//------------------------------------------------------------------------------
/**
    Этот метод вернет true, если текущее время оценки больше.
    затем время окончания работы.
*/
bool
AnimJob::IsExpired(Timing::Tick time) const
{
    if (this->IsInfinite())
    {
        return false;
    }
    else
    {
        return (time >= (this->baseTime + this->startTime + this->duration));
    }
}

//------------------------------------------------------------------------------
/**
    Возвращает абсолютное время начала (BaseTime + StartTime).
*/
Timing::Tick
AnimJob::GetAbsoluteStartTime() const
{
    return (this->baseTime + this->startTime);
}

//------------------------------------------------------------------------------
/**
    Return the absolute end time (BaseTime + StartTime + Duration).
    Method fails hard if AnimJob is infinite.
*/
Timing::Tick
AnimJob::GetAbsoluteEndTime() const
{
    n_assert(!this->IsInfinite());
    return (this->baseTime + this->startTime + this->duration);
}

//------------------------------------------------------------------------------
/**
    Возвращает абсолютное время окончания перед началом фазы затухания.
    ((BaseTime + StartTime + Duration) – FadeOut)
*/
Timing::Tick
AnimJob::GetAbsoluteStopTime() const
{
    n_assert(!this->IsInfinite());
    return (this->baseTime + this->startTime + this->duration) - this->fadeOutTime;
}

//------------------------------------------------------------------------------
/**
    This is a helper method for subclasses and returns the current blend
    weight for the current relative evaluation time, taking the fade-in
    and fade-out phases into account.
*/
float
AnimJob::ComputeBlendWeight(Timing::Tick relEvalTime) const
{
    n_assert(relEvalTime >= 0);

    // проверь, прошло ли время затухания
    Timing::Tick fadeOutStartTime = this->duration - this->fadeOutTime;
    if (!this->IsInfinite() && (this->fadeOutTime > 0) && (relEvalTime > fadeOutStartTime))
    {
        // мы находимся на этапе затухания
        float fadeOutWeight = this->blendWeight * (1.0f - (float(relEvalTime - fadeOutStartTime) / float(fadeOutTime)));
        return fadeOutWeight;
    }
    else if ((fadeInTime > 0) && (relEvalTime < this->fadeInTime))
    {
        // мы находимся в фазе затухания
        float fadeInWeight = this->blendWeight * (float(relEvalTime) / float(fadeInTime));
        return fadeInWeight;
    }
    else
    {
        // мы находимся между фазой затухания и затухания
        return this->blendWeight;
    }
}

//------------------------------------------------------------------------------
/**
    Updates evaluation times. 
    Must be done every frame, even if character is not visible and so animjob is not evaluated.
*/
void 
AnimJob::UpdateTimes(Timing::Tick time)
{
    this->curRelEvalTime = time - (this->baseTime + this->startTime);
    Timing::Tick frameTicks = this->curRelEvalTime - this->lastRelEvalTime;
    this->lastRelEvalTime = this->curRelEvalTime;

    if (!this->isPaused)
    {   
        int timeDivider = Math::n_frnd(1 / this->timeFactor);
        Timing::Tick timeDiff = frameTicks / timeDivider;
        this->curSampleTime  = this->lastSampleTime + timeDiff;
        this->lastSampleTime = this->curSampleTime;
        this->curSampleTime += this->timeOffset;
    }
}

//------------------------------------------------------------------------------
/**
    Остановите работу анимации в указанное время. Это просто обновит
    член продолжительности.
*/
void
AnimJob::Stop(Timing::Tick time)
{
    // остановимся только в том случае, если мы еще не остановились, иначе мы можем
    // перемещать фактическое время остановки все дальше и дальше в будущее
    if (!this->IsStoppingOrExpired(time))
    {
        // преобразовать абсолютное текущее время в относительное время анимационного задания и добавить время затухания
        Timing::Tick newEndTime = (time - this->baseTime) + this->fadeOutTime;

        // вычислить новую продолжительность задания анимации        
        this->duration = newEndTime - this->startTime;
        //FIXME, почему это утверждение
        //n_assert(this->duration >= 0);
    }
}

//------------------------------------------------------------------------------
/**
    Приостановить работу анимации в заданное время. Возобновляет паузу, если она уже была приостановлена.
*/
void 
AnimJob::Pause(bool pause)
{
    // переключить паузу
    this->isPaused = pause;

    if (this->isPaused)
    {
        this->lastRelEvalTime = this->curRelEvalTime;
        this->lastSampleTime = this->curSampleTime;
    }
}

//------------------------------------------------------------------------------
/**
    This method is called by the AnimSequencer when this job is active
    (the current eval time is between the start and end time of the job).
    The AnimJob object is expected to fill the provided AnimSampleBuffer 
    with a result (sampled keys and sample counts, the sample counts indicate 
    whether a given sample contributes to the final blended result. If the mixIn
    pointer is valid, the method must perform animation mixing as well.

    This method is usually implemented by subclasses.
*/
Ptr<Job>
AnimJob::CreateEvaluationJob(Timing::Tick time, const Ptr<AnimSampleBuffer>& optMixIn, const Ptr<AnimSampleBuffer>& result)
{
    // реализовать в подклассе!
    return Ptr<Job>();
}

//------------------------------------------------------------------------------
/**
*/
Util::Array<AnimEventInfo> 
AnimJob::EmitAnimEvents(Timing::Tick startTime, Timing::Tick endTime, const Util::String& optionalCatgeory)
{
    // реализовать в подклассе
    Util::Array<AnimEventInfo> result;
    return result;
}


} // пространство имен Анимация
