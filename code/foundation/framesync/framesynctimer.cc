//------------------------------------------------------------------------------
//  framesynctimer.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "framesync/framesynctimer.h"

namespace FrameSync
{
__ImplementClass(FrameSync::FrameSyncTimer, 'FSYT', Core::RefCounted);
__ImplementSingleton(FrameSync::FrameSyncTimer);

using namespace Timing;

//------------------------------------------------------------------------------
/**
*/
FrameSyncTimer::FrameSyncTimer() :
    time(0.0),
    ticks(0),
    frameTime(0.0),
    frameTicks(0),
    scaledTime(0.0),
    timeFactor(1.0),
    realTime(0.0),
    frameIndex(0),
    isValid(false)   
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
FrameSyncTimer::~FrameSyncTimer()
{
    if (this->IsValid())
    {
        this->Discard();
    }
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
    Запрашивать основное время из FrameSyncHandlerThread один раз. 
    время создания. Это безопасно, но медленно, но необходимо немедленно
    укажите действительную отметку времени в первом кадре.
*/
void
FrameSyncTimer::Setup()
{
    n_assert(!this->isValid);
    this->isValid = true;

    // сделать медленную инициализацию в первый раз
    this->UpdateTimePolling();

    // исправить время кадра до разумного значения
    this->frameTime = 1.0 / 60.0;
    this->frameTicks = Timing::SecondsToTicks(this->frameTime);

    // запустить основной таймер
    this->masterTimer.Start();
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSyncTimer::Discard()
{
    n_assert(this->isValid);
    this->isValid = false;
}

//------------------------------------------------------------------------------
/**
    Update the time by polling the FrameSyncHandlerThread singleton.
    This requires a CriticalSection to be taken and should not be used
    for threads which run in lockstep mode (i.e. the game thread or the
    render thread) since these will update their time automatically
    during the frame-sync-point.
*/
void
FrameSyncTimer::UpdateTimePolling()
{
    this->Update(this->masterTimer.GetTime());
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSyncTimer::StartTime()
{
    this->masterTimer.Start();  
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSyncTimer::StopTime()
{
    this->masterTimer.Stop();
}

//------------------------------------------------------------------------------
/**
*/
void
FrameSyncTimer::ResetTime()
{
    this->masterTimer.Reset();
}

//------------------------------------------------------------------------------
/**
*/
bool
FrameSyncTimer::IsTimeRunning() const
{
    return this->masterTimer.Running();
}

//------------------------------------------------------------------------------
/**
    Этот метод будет вызываться один раз для каждого кадра в точке синхронизации кадров.
    из FrameSyncHandlerThread. Обратите внимание, что этот метод всегда будет 
    вызывается из локального потока, а не из другого потока, поэтому мы не
    нужно заботиться о потокобезопасности.
*/
void
FrameSyncTimer::Update(Time newTime)
{
    this->frameTime = newTime - this->time; 
    this->scaledTime = this->scaledTime + this->frameTime * this->timeFactor;    
    if (this->frameTime < 0.001)
    {
        this->frameTime = 0.001;
    }
    this->time = newTime;

    Tick newTicks = Timing::SecondsToTicks(newTime);
    this->frameTicks = newTicks - this->ticks;
    if (this->frameTicks < 1)
    {
        this->frameTicks = 1;
    }
    this->ticks = newTicks;

    this->frameIndex++;
}

//------------------------------------------------------------------------------
/**
*/
Timing::Time 
FrameSyncTimer::GetScaledFrameTime() const
{
    return this->frameTime * this->timeFactor;
}

//------------------------------------------------------------------------------
/**
*/
IndexT
FrameSyncTimer::GetFrameIndex() const
{
    return this->frameIndex;
}

} // пространство имен FrameSync
