#pragma once
#include <atomic>
namespace Threading
{
class AssertingMutex
{
public:
    AssertingMutex();
    ~AssertingMutex();
    void Lock();
    void Unlock();
private:
    std::atomic_int locked;
};
struct AssertingScope
{
    AssertingScope(AssertingMutex* mutex)
        : mutex(mutex)
    {
        mutex->Lock();
    }
    ~AssertingScope()
    {
        mutex->Unlock();
    }
    AssertingMutex* mutex;
};
}