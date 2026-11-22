#pragma once
#include "core/types.h"
#include "threading/criticalsection.h"
namespace Win32
{
class Win32ThreadBarrier
{
public:
    Win32ThreadBarrier();
    ~Win32ThreadBarrier();
    void Setup(SizeT numThreads);
    bool IsValid() const;
    bool Arrive();
    void Wait();
    void SignalContinue();
private:
    Threading::CriticalSection critSect;
    long numThreads;
    volatile long outstandingThreads;
    HANDLE event;
    bool isValid;
};
inline
Win32ThreadBarrier::Win32ThreadBarrier() :
    numThreads(0),
    outstandingThreads(0),
    isValid(false)
{
    this->event = CreateEvent(NULL, TRUE, FALSE, NULL);
}
inline
Win32ThreadBarrier::~Win32ThreadBarrier()
{
    CloseHandle(this->event);
    this->event = 0;
}
inline void
Win32ThreadBarrier::Setup(SizeT num)
{
    n_assert(!this->isValid);
    this->numThreads = num;
    this->outstandingThreads = num;
    this->isValid = true;
}
inline bool
Win32ThreadBarrier::IsValid() const
{
    return this->isValid;
}
inline bool
Win32ThreadBarrier::Arrive()
{
    this->critSect.Enter();
    n_assert(this->outstandingThreads > 0);
    this->outstandingThreads--;
    return (0 == this->outstandingThreads);
}
inline void
Win32ThreadBarrier::Wait()
{
    ResetEvent(this->event);
    this->critSect.Leave();
    DWORD reason = WaitForSingleObject(this->event, 2000);
    if (WAIT_TIMEOUT == reason)
    {
        n_printf("Время ожидания Win32ThreadBarrier::Wait() истекло!\n");
    }
}
inline void
Win32ThreadBarrier::SignalContinue()
{
    this->outstandingThreads = this->numThreads;
    SetEvent(this->event);
    this->critSect.Leave();
}
}