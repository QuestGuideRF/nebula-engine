#pragma once
#include "core/types.h"
namespace Linux
{
class LinuxCompletionCounter
{
public:
    LinuxCompletionCounter();
    ~LinuxCompletionCounter();
    void Reset(int count);
    bool Decrement(int num);
    void Wait();
    bool Peek();
private:
    volatile int curCount;
    pthread_cond_t completionEvent;
};
inline
LinuxCompletionCounter::LinuxCompletionCounter() :
    curCount(0)
{
    pthread_cond_init(&this->completionEvent, 0);
}
inline
LinuxCompletionCounter::~LinuxCompletionCounter()
{
    pthread_cond_destroy(&this->completionEvent);
}
inline void
LinuxCompletionCounter::Reset(int count)
{
    n_assert(count > 0);
    n_assert(0 == this->curCount);
    this->curCount = count;
}
inline bool
LinuxCompletionCounter::Decrement(int num)
{
    if (LinuxInterlocked::Add(this->curCount, -num) == num)
    {
        pthread_cond_signal(&this->completionEvent);
        return true;
    }
    else
    {
        return false;
    }
}
inline void
LinuxCompletionCounter::Wait()
{
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m);
    while(this->curCount > 0)
    {
        pthread_cond_wait(&this->completionEvent, &m);
    }
    pthread_mutex_unlock(&m);
    pthread_mutex_destroy(&m);
}
inline bool
LinuxCompletionCounter::Peek()
{
    return (0 == this->curCount);
}
}