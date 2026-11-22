#pragma once
#include "core/types.h"
#include "core/sysfunc.h"
#define NEBULA_USER_CRITICAL_SECTION 1
#if NEBULA_USER_CRITICAL_SECTION
extern "C" void _WriteBarrier();
extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_WriteBarrier)
#pragma intrinsic(_ReadWriteBarrier)
#endif
namespace Win32
{
class Win32CriticalSection
{
public:
    Win32CriticalSection();
    ~Win32CriticalSection();
    void Enter() const;
    void Leave() const;
private:
#if NEBULA_USER_CRITICAL_SECTION
    bool PerfLockImmediate(DWORD dwThreadID) const;
    bool PerfLock(DWORD dwThreadID) const;
    bool PerfLockKernel(DWORD dwThreadID) const;
    void PerfUnlock() const;
    void WaiterPlus() const;
    void WaiterMinus() const;
    void AllocateKernelSemaphore() const;
    mutable volatile DWORD  lockerThread;
    volatile DWORD  spinMax;
    mutable volatile long   waiterCount;
    mutable volatile HANDLE semaphore;
    mutable uint recursiveLockCount;
#else
    mutable CRITICAL_SECTION criticalSection;
#endif
};
#if !NEBULA_USER_CRITICAL_SECTION
inline
Win32CriticalSection::Win32CriticalSection()
{
    InitializeCriticalSectionAndSpinCount(&this->criticalSection, 4096);
}
inline
Win32CriticalSection::~Win32CriticalSection()
{
    DeleteCriticalSection(&this->criticalSection);
}
inline void
Win32CriticalSection::Enter() const
{
    EnterCriticalSection(&this->criticalSection);
}
inline void
Win32CriticalSection::Leave() const
{
    LeaveCriticalSection(&this->criticalSection);
}
#else
inline void
Win32CriticalSection::WaiterPlus() const
{
    _InterlockedIncrement(&this->waiterCount);
}
inline void
Win32CriticalSection::WaiterMinus() const
{
    _InterlockedDecrement(&this->waiterCount);
}
inline bool
Win32CriticalSection::PerfLockImmediate(DWORD dwThreadID) const
{
    return (0 == InterlockedCompareExchange((long*) &this->lockerThread, dwThreadID, 0));
}
inline void
Win32CriticalSection::Enter() const
{
    DWORD threadId = GetCurrentThreadId();
    if (threadId != this->lockerThread)
    {
        if ((this->lockerThread == 0) &&
            this->PerfLockImmediate(threadId))
        {
        }
        else
        {
            this->PerfLock(threadId);
        }
    }
    this->recursiveLockCount++;
}
inline void
Win32CriticalSection::AllocateKernelSemaphore() const
{
    if (!this->semaphore)
    {
        HANDLE handle = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
        n_assert(NULL != handle);
        if (InterlockedCompareExchangePointer(&this->semaphore, handle, NULL))
            CloseHandle(handle);
    }
}
inline bool
Win32CriticalSection::PerfLock(DWORD dwThreadID) const
{
    for (DWORD dwSpin = 0; dwSpin < spinMax; dwSpin++)
    {
        if (this->PerfLockImmediate(dwThreadID))
            return true;
        YieldProcessor();
    }
    this->AllocateKernelSemaphore();
    bool result = this->PerfLockKernel(dwThreadID);
    this->WaiterMinus();
    return result;
}
inline bool
Win32CriticalSection::PerfLockKernel(DWORD dwThreadID) const
{
    bool waiter = false;
    for (;;)
    {
        if (!waiter)
            this->WaiterPlus();
        if (PerfLockImmediate(dwThreadID))
            return true;
        n_assert(this->semaphore);
        switch (WaitForSingleObject(this->semaphore, INFINITE))
        {
        case WAIT_OBJECT_0:
            waiter = false;
            break;
        case WAIT_TIMEOUT:
            waiter = true;
            break;
        default:
            break;
        };
    }
}
inline void
Win32CriticalSection::Leave() const
{
    if (--recursiveLockCount == 0)
    {
        this->PerfUnlock();
    }
}
inline void
Win32CriticalSection::PerfUnlock() const
{
    _WriteBarrier();
    this->lockerThread = 0;
    _ReadWriteBarrier();
    if (this->waiterCount > 0)
    {
        WaiterMinus();
        ReleaseSemaphore(this->semaphore, 1, NULL);
    }
}
#endif
};