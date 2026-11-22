#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "timing/time.h"
#include "timing/timer.h"
namespace FrameSync
{
class FrameSyncTimer : public Core::RefCounted
{
    __DeclareClass(FrameSyncTimer);
    __DeclareSingleton(FrameSyncTimer);
public:
    FrameSyncTimer();
    virtual ~FrameSyncTimer();
    void Setup();
    void Discard();
    bool IsValid() const;
    void UpdateTimePolling();
    void StartTime();
    void StopTime();
    void ResetTime();
    bool IsTimeRunning() const;
    Timing::Time GetTime() const;
    Timing::Tick GetTicks() const;
    Timing::Time GetFrameTime() const;
    Timing::Tick GetFrameTicks() const;
    IndexT GetFrameIndex() const;
    void SetTimeFactor(Timing::Time factor);
    Timing::Time GetTimeFactor() const;
    Timing::Time GetScaledTime() const;
    Timing::Time GetScaledFrameTime() const;
private:
    friend class FrameSyncHandlerThread;
    void Update(Timing::Time masterTime);
    Timing::Time time;
    Timing::Tick ticks;
    Timing::Time frameTime;
    Timing::Tick frameTicks;
    Timing::Time scaledTime;
    Timing::Time timeFactor;
    Timing::Time realTime;
    Timing::Timer masterTimer;
    IndexT frameIndex;
    bool isValid;
};
inline bool
FrameSyncTimer::IsValid() const
{
    return this->isValid;
}
inline Timing::Time
FrameSyncTimer::GetTime() const
{
    return this->time;
}
inline Timing::Tick
FrameSyncTimer::GetTicks() const
{
    return this->ticks;
}
inline Timing::Time
FrameSyncTimer::GetFrameTime() const
{
    return this->frameTime;
}
inline Timing::Tick
FrameSyncTimer::GetFrameTicks() const
{
    return this->frameTicks;
}
inline void
FrameSyncTimer::SetTimeFactor(Timing::Time factor)
{
    this->timeFactor = factor;
}
inline Timing::Time
FrameSyncTimer::GetTimeFactor() const
{
    return this->timeFactor;
}
inline Timing::Time
FrameSyncTimer::GetScaledTime()const
{
    return this->scaledTime;
}
}