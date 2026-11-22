#pragma once
#include "core/types.h"
#include "threading/criticalsection.h"
#include <semaphore.h>
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
    sem_t* semaphore;
    bool isValid;
};
PosixThreadBarrier::PosixThreadBarrier() :
    numThreads(0),
    outstandingThreads(0),
    isValid(false)
{
    this->semaphore = new sem_t;
    sem_init(this->semaphore, 0, 0);
}
PosixThreadBarrier::~PosixThreadBarrier()
{
    sem_destroy(this->semaphore);
    delete this->semaphore;
    this->semaphore = 0;
}
void
PosixThreadBarrier::Setup(SizeT numThreads)
{
    n_assert(!this->IsValid());
    this->numThreads = numThreads;
    this->outstandingThreads = numThreads;
    this->isValid = true;
}
bool
PosixThreadBarrier::IsValid() const
{
    return this->isValid;
}
bool
PosixThreadBarrier::Arrive()
{
    this->critSect.Enter();
    n_assert(this->outstandingThreads > 0);
    this->outstandingThreads--;
    return (0 == this->outstandingThreads);
}
void
PosixThreadBarrier::Wait()
{
    this->critSect.Leave();
    int timeout = 2000;
    while (timeout > 0)
    {
        if (0 == sem_trywait(this->semaphore))
        {
            return;
        }
        usleep(1000);
        timeout -= 1;
    }
}
void
PosixThreadBarrier::SignalContinue()
{
    this->outstandingThreads = this->numThreads;
    sem_post(this->semaphore);
    this->critSect.Leave();
}
}