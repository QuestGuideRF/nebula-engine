#pragma once
#include "core/types.h"
#include "threading/criticalsection.h"
#include <pthread.h>
#include <time.h>
namespace Posix
{
class PosixThreadBarrier
{
public:
    PosixThreadBarrier();
    ~PosixThreadBarrier();
    void Setup(SizeT numThreads);
    bool IsValid() const;
    bool Arrive();
    void Wait();
    void SignalContinue();
private:
    Threading::CriticalSection critSect;
    long numThreads;
    volatile long outstandingThreads;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    bool isValid;
    timespec ts;
};
inline
PosixThreadBarrier::PosixThreadBarrier() :
    numThreads(0),
    outstandingThreads(0),
    isValid(false)
{
    pthread_cond_init(&cond,NULL);
    pthread_mutex_init(&mutex,NULL);
}
inline
PosixThreadBarrier::~PosixThreadBarrier()
{
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
inline void
PosixThreadBarrier::Setup(SizeT num)
{
    n_assert(!this->isValid);
    this->numThreads = num;
    this->outstandingThreads = num;
    this->isValid = true;
}
inline bool
PosixThreadBarrier::IsValid() const
{
    return this->isValid;
}
inline bool
PosixThreadBarrier::Arrive()
{
    this->critSect.Enter();
    n_assert(this->outstandingThreads > 0);
    this->outstandingThreads--;
    return (0 == this->outstandingThreads);
}
inline void
PosixThreadBarrier::Wait()
{
    pthread_mutex_lock(&mutex);
    this->critSect.Leave();
    clock_gettime(CLOCK_REALTIME,&ts);
    ts.tv_sec += 2;
    int reason = pthread_cond_timedwait(&cond,&mutex,&ts);
    if ( ETIMEDOUT == reason )
    {
        n_printf("Время ожидания PosixThreadBarrier::Wait() истекло!\n");
    }
}
inline void
PosixThreadBarrier::SignalContinue()
{
    this->outstandingThreads = this->numThreads;
    pthread_cond_broadcast(&cond);
    this->critSect.Leave();
}
}