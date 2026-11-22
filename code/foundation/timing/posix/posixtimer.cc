//------------------------------------------------------------------------------
//  posixtimer.cc
//  (C) 2007 Олег Хриптул (Харон)
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "timing/posix/posixtimer.h"
#include <time.h>

namespace Posix
{

//------------------------------------------------------------------------------
/**
*/
PosixTimer::PosixTimer() :
    running(false),
    diffTime(0),
    stopTime(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Start the timer. This will update the diffTime member to reflect
    the accumulated time when the timer was not running (basically the
    difference between this timer's time and the real system time).
*/
void
PosixTimer::Start()
{
    n_assert(!this->running);

    // query the current real time and update the diffTime member
    // учитывать «потерянное» время с момента остановки таймера
    timespec times;
    n_assert(clock_gettime(CLOCK_MONOTONIC,&times) == 0);
    
    Timing::Time curRealTime;
    curRealTime = ToTime(times);
    
    this->diffTime += curRealTime - this->stopTime;
    this->stopTime = 0;
    this->running = true;
}

//------------------------------------------------------------------------------
/**
    Stop the timer. This will record the current realtime, so that
    the next Start() can measure the time lost between Stop() and Start()
    which must be taken into account to keep track of the difference between
    this timer's time and realtime.
*/
void
PosixTimer::Stop()
{
    n_assert(this->running);
    this->stopTime = clock();
    this->running = false;
}

//------------------------------------------------------------------------------
/**
    Сбросьте таймер, чтобы он снова начал отсчет с нуля.
*/
void
PosixTimer::Reset()
{
    bool wasRunning = this->running;
    if (wasRunning)
    {
        this->Stop();
    }
    this->stopTime = 0;
    this->diffTime = 0;
    if (wasRunning)
    {
        this->Start();
    }
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если таймер в данный момент работает.
*/
bool
PosixTimer::Running() const
{
    return this->running;
}

//------------------------------------------------------------------------------
/**
    Преобразование timespec в Timing::Time
*/
Timing::Time 
PosixTimer::ToTime(const timespec & ts) const
{
    return (double)ts.tv_sec + (double)ts.tv_nsec * 0.000000001;
} 

//------------------------------------------------------------------------------
/**
    Это возвращает внутреннее местное время в виде большого целого числа.
*/
Timing::Time
PosixTimer::InternalTime() const
{
    // получить текущее время в реальном времени    
    Timing::Time ret;
    if (this->running)
    {
        // мы работаем, запросить текущее время
        timespec tp;
        n_assert(clock_gettime(CLOCK_MONOTONIC,&tp) == 0);
        ret = ToTime(tp);
    }
    else
    {
        // мы остановились, используйте время на последней остановке
        ret = this->stopTime;
    }

    // перевести в местное время
    ret -= this->diffTime;

    return ret;
}

//------------------------------------------------------------------------------
/**
    Это возвращает текущее время таймера в секундах.
*/
Timing::Time
PosixTimer::GetTime() const
{
    Timing::Time seconds = this->InternalTime();
    return seconds;
}

//------------------------------------------------------------------------------
/**
    Это возвращает текущее время таймера в «тиках».
*/
Timing::Tick
PosixTimer::GetTicks() const
{
    return Timing::SecondsToTicks(this->InternalTime());   
}

} // пространство имен Posix

