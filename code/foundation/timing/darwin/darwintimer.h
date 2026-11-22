#pragma once
#ifndef DARWIN_DARWINTIMER_H
#define DARWIN_DARWINTIMER_H
#include "core/types.h"
#include "timing/time.h"
namespace Darwin
{
class DarwinTimer
{
public:
    DarwinTimer();
    void Start();
    void Stop();
    void Reset();
    bool Running() const;
    Timing::Time GetTime() const;
    uint GetTicks() const;
private:
    uint64_t InternalTime() const;
    bool running;
    uint64_t diffTime;
    uint64_t stopTime;
};
}
#endif