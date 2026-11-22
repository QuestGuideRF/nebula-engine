#pragma once
#ifndef WIN32_WIN32TIMER_H
#define WIN32_WIN32TIMER_H
#include "core/types.h"
#include "timing/time.h"
namespace Win32
{
class Win32Timer
{
public:
    Win32Timer();
    void Start();
    void Stop();
    void Reset();
    bool Running() const;
    Timing::Time GetTime() const;
    Timing::Tick GetTicks() const;
private:
    __int64 InternalTime() const;
    bool running;
    __int64 diffTime;
    __int64 stopTime;
};
}
#endif