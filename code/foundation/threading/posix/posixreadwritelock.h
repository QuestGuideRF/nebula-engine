#pragma once
#include <pthread.h>
#include "threading/threadid.h"
namespace Posix
{
class PosixReadWriteLock
{
public:
    PosixReadWriteLock();
    ~PosixReadWriteLock();
    void LockRead();
    void LockWrite();
    void UnlockRead();
    void UnlockWrite();
private:
    int32_t readCounter = 0;
    int32_t writeCounter = 0;
    pthread_rwlock_t lock;
    Threading::ThreadId lockingThread = 0;
};
}