#pragma once
#include "core/types.h"
namespace OSX
{
class OSXCriticalSection
{
public:
    OSXCriticalSection();
    ~OSXCriticalSection();
    void Enter() const;
    void Leave() const;
private:
    mutable pthread_mutex_t mutex;
};
inline
OSXCriticalSection::OSXCriticalSection()
{
    pthread_mutexattr_t mutexAttrs;
    pthread_mutexattr_init(&mutexAttrs);
    pthread_mutexattr_settype(&mutexAttrs, PTHREAD_MUTEX_RECURSIVE);
    int res = pthread_mutex_init(&this->mutex, &mutexAttrs);
    n_assert(0 == res);
    pthread_mutexattr_destroy(&mutexAttrs);
}
inline
OSXCriticalSection::~OSXCriticalSection()
{
    int res = pthread_mutex_destroy(&this->mutex);
    n_assert(0 == res);
}
inline void
OSXCriticalSection::Enter() const
{
    pthread_mutex_lock(&this->mutex);
}
inline void
OSXCriticalSection::Leave() const
{
    pthread_mutex_unlock(&this->mutex);
}
}