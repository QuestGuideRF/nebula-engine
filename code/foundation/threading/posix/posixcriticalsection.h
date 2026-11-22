#pragma once
#ifndef THREADING_POSIXCRITICALSECTION_H
#define THREADING_POSIXCRITICALSECTION_H
#include "core/types.h"
#include <pthread.h>
namespace Posix
{
class PosixCriticalSection
{
public:
    PosixCriticalSection();
    ~PosixCriticalSection();
    void operator=(PosixCriticalSection&& rhs);
    void Enter() const;
    void Leave() const;
private:
    pthread_mutex_t* mutex;
};
inline
PosixCriticalSection::PosixCriticalSection() :
    mutex(0)
{
    this->mutex = new pthread_mutex_t;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(this->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}
inline
PosixCriticalSection::~PosixCriticalSection()
{
    pthread_mutex_destroy(this->mutex);
    delete this->mutex;
    this->mutex = 0;
}
inline void
PosixCriticalSection::operator=(PosixCriticalSection&& rhs)
{
    if (this->mutex)
    {
        pthread_mutex_destroy(this->mutex);
        delete this->mutex;
    }
    this->mutex = rhs.mutex;
    rhs.mutex = nullptr;
}
inline
void
PosixCriticalSection::Enter() const
{
    pthread_mutex_lock(this->mutex);
}
inline
void
PosixCriticalSection::Leave() const
{
    pthread_mutex_unlock(this->mutex);
}
};
#endif