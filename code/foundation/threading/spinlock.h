#pragma once
#include "threading/interlocked.h"
#include "threading/thread.h"
namespace Threading
{
class Spinlock
{
public:
    Spinlock();
    ~Spinlock();
    void operator=(Spinlock&& rhs);
    void Lock();
    void Unlock();
private:
    volatile Threading::ThreadId lock;
};
inline
Spinlock::Spinlock()
    : lock(InvalidThreadId)
{
}
inline
Spinlock::~Spinlock()
{
    this->Lock();
    this->lock = InvalidThreadId;
}
inline void
Spinlock::operator=(Spinlock&& rhs)
{
    rhs.Lock();
    rhs.lock = InvalidThreadId;
    if (InvalidThreadId == this->lock)
        return;
    while (Interlocked::CompareExchange((volatile ThreadIdStorage*)&this->lock, InvalidThreadId, InvalidThreadId) != InvalidThreadId)
    {
        Thread::YieldThread();
    }
}
inline void
Spinlock::Lock()
{
    ThreadId threadId = Thread::GetMyThreadId();
    if (threadId == this->lock)
        return;
    while (Interlocked::CompareExchange((volatile ThreadIdStorage*) &this->lock, threadId, InvalidThreadId) != InvalidThreadId)
    {
        Thread::YieldThread();
    }
}
inline void
Spinlock::Unlock()
{
    n_assert(this->lock != InvalidThreadId);
    Threading::Interlocked::Exchange((volatile ThreadIdStorage*) &this->lock, InvalidThreadId);
}
struct SpinlockScope
{
    SpinlockScope(Threading::Spinlock* lock)
        : lock(lock)
    {
        this->lock->Lock();
    }
    ~SpinlockScope()
    {
        this->lock->Unlock();
        this->lock = nullptr;
    }
    Threading::Spinlock* lock;
};
}