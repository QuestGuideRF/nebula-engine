#pragma once
#include "core/types.h"
namespace Gcc
{
class GccInterlocked
{
public:
    static int Increment(int volatile* var);
    static int Decrement(int volatile* var);
    static long Increment(long volatile* var);
    static long Decrement(long volatile* var);
    static int Add(int volatile* var, int add);
    static int Or(int volatile* var, int value);
    static int And(int volatile* var, int value);
    static int Xor(int volatile* var, int value);
    static int Exchange(int volatile* dest, int value);
    static void* ExchangePointer(void* volatile* dest, void* value);
    static int CompareExchange(int volatile* dest, int exchange, int comparand);
    static void* CompareExchangePointer(void* volatile* dest, void* exchange, void* comparand);
};
inline int
GccInterlocked::Increment(int volatile* var)
{
    return __sync_add_and_fetch(var, 1);
}
inline int
GccInterlocked::Decrement(int volatile* var)
{
    return __sync_sub_and_fetch(var, 1);
}
inline long
GccInterlocked::Increment(long volatile* var)
{
    return __sync_add_and_fetch(var, 1);
}
inline long
GccInterlocked::Decrement(long volatile* var)
{
    return __sync_sub_and_fetch(var, 1);
}
inline int
GccInterlocked::Add(int volatile* var, int add)
{
    return __sync_fetch_and_add(var, add);
}
inline int
GccInterlocked::Or(int volatile* var, int add)
{
    return __sync_fetch_and_or(var, add);
}
inline int
GccInterlocked::And(int volatile* var, int add)
{
    return __sync_fetch_and_and(var, add);
}
inline int
GccInterlocked::Xor(int volatile* var, int add)
{
    return __sync_fetch_and_xor(var, add);
}
inline int
GccInterlocked::Exchange(int volatile* dest, int value)
{
    return __sync_lock_test_and_set(dest, value);
}
inline void*
GccInterlocked::ExchangePointer(void* volatile* dest, void* value)
{
    return __sync_lock_test_and_set(dest, value);
}
inline int
GccInterlocked::CompareExchange(int volatile* dest, int exchange, int comparand)
{
    return __sync_val_compare_and_swap(dest, comparand, exchange);
}
inline void*
GccInterlocked::CompareExchangePointer(void* volatile* dest, void* exchange, void* comparand)
{
    return __sync_val_compare_and_swap(dest, comparand, exchange);
}
}