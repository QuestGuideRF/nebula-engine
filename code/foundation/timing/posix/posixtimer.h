#pragma once
#ifndef POSIX_POSIXTIMER_H
#define POSIX_POSIXTIMER_H
#include "core/types.h"
#include "timing/time.h"
namespace Posix
{
class PosixTimer
{
public:
    PosixTimer();
    void Start();
    void Stop();
    void Reset();
    bool Running() const;
    Timing::Time GetTime() const;
    Timing::Tick GetTicks() const;
private:
    Timing::Time InternalTime() const;
    Timing::Time ToTime(const timespec & ts) const;
    bool running;
    Timing::Time diffTime;
    Timing::Time stopTime;
};
}
#endif