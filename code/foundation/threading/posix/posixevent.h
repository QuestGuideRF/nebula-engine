#pragma once
#ifndef POSIX_POSIXEVENT_H
#define POSIX_POSIXEVENT_H
#include "core/types.h"
#include <semaphore.h>
namespace Posix
{
class PosixEvent
{
public:
    PosixEvent(bool manualReset=false);
    PosixEvent(PosixEvent&& ev);
    ~PosixEvent();
    void Signal();
    void Reset();
    void Wait() const;
    bool WaitTimeout(int ms) const;
    bool Peek() const;
    bool IsManual() const;
private:
    bool manual;
    sem_t* semaphore;
};
inline
PosixEvent::PosixEvent(bool manualReset)
    : semaphore(0)
    , manual(manualReset)
{
    this->semaphore = new sem_t;
    sem_init(this->semaphore, 0, 0);
}
inline
PosixEvent::PosixEvent(PosixEvent&& ev)
{
    this->semaphore = ev.semaphore;
    this->manual = ev.semaphore;
    ev.semaphore = nullptr;
}
inline
PosixEvent::~PosixEvent()
{
    sem_destroy(this->semaphore);
    delete this->semaphore;
    this->semaphore = 0;
}
inline void
PosixEvent::Signal()
{
    sem_post(this->semaphore);
}
inline void
PosixEvent::Reset()
{
}
inline void
PosixEvent::Wait() const
{
    sem_wait(this->semaphore);
}
inline bool
PosixEvent::WaitTimeout(int timeoutInMilliSec) const
{
    while (timeoutInMilliSec > 0)
    {
        if (0 == sem_trywait(this->semaphore))
        {
            return true;
        }
        usleep(1000);
        timeoutInMilliSec -= 1;
    }
    return false;
}
inline bool
PosixEvent::Peek() const
{
    return 0 == sem_trywait(this->semaphore);
}
inline bool
PosixEvent::IsManual() const
{
    return this->manual;
}
};
#endif