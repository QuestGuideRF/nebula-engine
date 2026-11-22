#pragma once
#include "core/types.h"
#include <bits/pthreadtypes.h>
namespace Linux
{
class LinuxEvent
{
public:
    LinuxEvent(bool manualReset=false);
    LinuxEvent(LinuxEvent&& ev);
    LinuxEvent(const LinuxEvent& ev) = default;
    ~LinuxEvent();
    void Signal();
    void Wait() const;
    bool WaitTimeout(int ms) const;
    bool Peek() const;
    void Reset();
    bool IsManual() const;
    LinuxEvent& operator=(const LinuxEvent& ev) = default;
private:
    enum EventStatus
    {
        SIGNAL_NONE = 0,
        SIGNAL_ONE = 1,
        SIGNAL_ALL = 2,
    };
    mutable pthread_mutex_t mutex;
    mutable pthread_cond_t cond;
    bool manualReset;
    volatile mutable EventStatus status;
};
inline
LinuxEvent::LinuxEvent(bool manual)
    : manualReset(manual)
    , status(SIGNAL_NONE)
{
    int res = pthread_mutex_init(&this->mutex, nullptr);
    n_assert(0 == res);
    res = pthread_cond_init(&this->cond, nullptr);
    n_assert(0 == res);
}
inline
LinuxEvent::LinuxEvent(LinuxEvent&& ev)
{
    int res = pthread_cond_destroy(&this->cond);
    n_assert(0 == res);
    res = pthread_mutex_destroy(&this->mutex);
    n_assert(0 == res);
    this->mutex = ev.mutex;
    this->cond = ev.cond;
    this->manualReset = ev.manualReset;
    this->status = ev.status;
    ev.mutex = pthread_mutex_t{};
    ev.cond = pthread_cond_t{};
    ev.status = SIGNAL_NONE;
}
inline
LinuxEvent::~LinuxEvent()
{
    int res = pthread_cond_destroy(&this->cond);
    n_assert(0 == res);
    res = pthread_mutex_destroy(&this->mutex);
    n_assert(0 == res);
}
inline void
LinuxEvent::Signal()
{
    pthread_mutex_lock(&this->mutex);
    if(this->manualReset)
    {
        this->status = SIGNAL_ALL;
        pthread_cond_broadcast(&this->cond);
    }
    else
    {
        this->status = SIGNAL_ONE;
        pthread_cond_signal(&this->cond);
    }
    pthread_mutex_unlock(&this->mutex);
}
inline void
LinuxEvent::Wait() const
{
    pthread_mutex_lock(&this->mutex);
    bool locked = false;
    do
    {
        if (this->status == SIGNAL_ONE)
        {
            this->status = SIGNAL_NONE;
            locked = true;
        }
        else if( this->status == SIGNAL_ALL)
        {
            locked = true;
        }
        else
        {
            int res = pthread_cond_wait(&this->cond, &this->mutex);
            n_assert(res == 0);
        }
    } while (!locked);
    pthread_mutex_unlock(&this->mutex);
}
inline bool
LinuxEvent::WaitTimeout(int ms) const
{
    bool timeOutOccured = false;
    pthread_mutex_lock(&this->mutex);
    bool locked = false;
    do
    {
        if (this->status == SIGNAL_ONE)
        {
            this->status = SIGNAL_NONE;
            locked = true;
        }
        else if( this->status == SIGNAL_ALL)
        {
            locked = true;
        }
        else
        {
            timespec timeSpec;
            timeSpec.tv_sec = ms / 1000;
            timeSpec.tv_nsec = (ms - timeSpec.tv_sec * 1000) * 1000000;
            int res = pthread_cond_timedwait(&this->cond, &this->mutex, &timeSpec);
            if (ETIMEDOUT == res)
            {
                timeOutOccured = true;
            }
        }
    } while(!locked && !timeOutOccured);
    pthread_mutex_unlock(&this->mutex);
    return !timeOutOccured;
}
inline bool
LinuxEvent::Peek() const
{
    return this->status != SIGNAL_NONE;
}
inline void
LinuxEvent::Reset()
{
    pthread_mutex_lock(&this->mutex);
    this->status = SIGNAL_NONE;
    pthread_mutex_unlock(&this->mutex);
}
inline bool
LinuxEvent::IsManual() const
{
    return this->manualReset;
}
}