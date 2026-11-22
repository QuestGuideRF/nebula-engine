#pragma once
#include "threading/interlocked.h"
namespace Threading
{
class SafeFlag
{
public:
    SafeFlag();
    void Set();
    void Clear();
    bool Test() const;
    bool TestAndClearIfSet();
private:
    int volatile flag;
};
inline
SafeFlag::SafeFlag() :
    flag(0)
{
}
inline void
SafeFlag::Set()
{
    Interlocked::Exchange(&this->flag, 1);
}
inline void
SafeFlag::Clear()
{
    Interlocked::Exchange(&this->flag, 0);
}
inline bool
SafeFlag::Test() const
{
    return (0 != this->flag);
}
inline bool
SafeFlag::TestAndClearIfSet()
{
    return (1 == Interlocked::CompareExchange(&this->flag, 0, 1));
}
}