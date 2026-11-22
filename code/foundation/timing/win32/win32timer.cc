//------------------------------------------------------------------------------
//  win32timer.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "timing/win32/win32timer.h"

namespace Win32
{

//------------------------------------------------------------------------------
/**
*/
Win32Timer::Win32Timer() :
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
Win32Timer::Start()
{
    n_assert(!this->running);
    
    // query the current real time and update the diffTime member
    // учитывать «потерянное» время с момента остановки таймера
    __int64 curRealTime;
    QueryPerformanceCounter((LARGE_INTEGER*) &curRealTime);
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
Win32Timer::Stop()
{
    n_assert(this->running);
    QueryPerformanceCounter((LARGE_INTEGER*) &this->stopTime);
    this->running = false;
}

//------------------------------------------------------------------------------
/**
    Сбросьте таймер, чтобы он снова начал отсчет с нуля.
*/
void
Win32Timer::Reset()
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
Win32Timer::Running() const
{
    return this->running;
}

//------------------------------------------------------------------------------
/**
    Это возвращает внутреннее местное время в виде большого целого числа.
*/
__int64
Win32Timer::InternalTime() const
{
    // получить текущее время в реальном времени
    __int64 time;
    if (this->running)
    {
        // мы работаем, запросить текущее время
        QueryPerformanceCounter((LARGE_INTEGER*) &time);
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
Win32Timer::GetTime() const
{
    // получить текущее время в реальном времени
    __int64 time = this->InternalTime();

    // запросить частоту производительности
    __int64 freq;
    QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
 
    // конвертировать в секунды
    Timing::Time seconds = ((Timing::Time)time) / ((Timing::Time)freq);
    return seconds;
}

//------------------------------------------------------------------------------
/**
    Это возвращает текущее время таймера в «тиках». Точка определена
    как одна миллисекунда (1/1000 секунды).
*/
Timing::Tick
Win32Timer::GetTicks() const
{
    // получить текущее время в реальном времени
    __int64 time = this->InternalTime();

    // запросить частоту производительности
    __int64 freq;
    QueryPerformanceFrequency((LARGE_INTEGER*) &freq);

    __int64 ticks64 = time / (freq / 1000);
    return (Timing::Tick) ticks64;
}

} // пространство имен Win32
