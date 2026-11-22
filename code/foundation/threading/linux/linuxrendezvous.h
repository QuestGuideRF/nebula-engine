#pragma once
#include "core/types.h"
#include <semaphore.h>
namespace Linux
{
class LinuxRendezvous
{
public:
    LinuxRendezvous();
    ~LinuxRendezvous();
    void MasterArrive();
    void SlaveArrive();
private:
    sem_t semMaster;
    sem_t semSlave;
};
inline
LinuxRendezvous::LinuxRendezvous()
{
    int res;
    res = sem_init(&this->semMaster, 0, 0);
    n_assert(0 == res);
    res = sem_init(&this->semSlave, 0, 0);
    n_assert(0 == res);
}
inline
LinuxRendezvous::~LinuxRendezvous()
{
    sem_destroy(&this->semMaster);
    sem_destroy(&this->semSlave);
}
inline void
LinuxRendezvous::MasterArrive()
{
    n_assert(sem_post(&this->semMaster) == 0);
    n_assert(sem_wait(&this->semSlave) == 0);
}
inline void
LinuxRendezvous::SlaveArrive()
{
    n_assert(sem_post(&this->semSlave) == 0);
    n_assert(sem_wait(&this->semMaster) == 0);
}
}