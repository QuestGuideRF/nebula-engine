//------------------------------------------------------------------------------
//  darwintimer.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "timing/darwin/darwintimer.h"

#include <mach/mach.h>
#include <mach/mach_time.h>

namespace Darwin
{

//------------------------------------------------------------------------------
/**
*/
DarwinTimer::DarwinTimer() :
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
DarwinTimer::Start()
{
    n_assert(!this->running);
    
    // query the current real time and update the diffTime member
    // учитывать «потерянное» время с момента остановки таймера
    uint64_t curRealTime = mach_absolute_time();
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
DarwinTimer::Stop()
{
    n_assert(this->running);
    this->stopTime = mach_absolute_time();
    this->running = false;
}

//------------------------------------------------------------------------------
/**
    Сбросьте таймер, чтобы он снова начал отсчет с нуля.
*/
void
DarwinTimer::Reset()
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
DarwinTimer::Running() const
{
    return this->running;
}

//------------------------------------------------------------------------------
/**
    Это возвращает внутреннее местное время в виде большого целого числа.
*/
uint64_t
DarwinTimer::InternalTime() const
{
    // получить текущее время в реальном времени
    uint64_t time;
    if (this->running)
    {
        // мы работаем, запросить текущее время
        time = mach_absolute_time();
    }
    else
    {
        // мы остановились, используйте время на последней остановке
        time = this->stopTime;
    }
 
    // перевести в местное время
    time -= this->diffTime;

    return time;
}

//------------------------------------------------------------------------------
/**
    Это возвращает текущее время таймера в секундах.
*/
Timing::Time
DarwinTimer::GetTime() const
{
    // получить текущее время в реальном времени
    uint64_t time = this->InternalTime();

    // запросить частоту производительности
    int64_t freq;
#if 0
    QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
#endif
 
    // конвертировать в секунды
    Timing::Time seconds = ((Timing::Time)time) / ((Timing::Time)freq);
    return seconds;
}

//------------------------------------------------------------------------------
/**
    Это возвращает текущее время таймера в «тиках».
*/
uint
DarwinTimer::GetTicks() const
{
    // получить текущее время в реальном времени
    uint64_t time = this->InternalTime();

    // запросить частоту производительности
    int64_t freq;
#if 0
    QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
#endif

    uint64_t ticks64 = time / (freq / 100000);
    return (uint) ticks64;
}

} // пространство имен Дарвина
