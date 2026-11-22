#pragma once
#include "core/types.h"
#include "core/config.h"
namespace Threading
{
using int64 = int64_t;
typedef volatile int AtomicCounter;
typedef volatile int64 AtomicCounter64;
namespace Interlocked
{
using int64 = int64_t;
int Add(int volatile* var, int add);
int64 Add(int64 volatile* var, int64 add);
int Or(int volatile* var, int value);
int64 Or(int64 volatile* var, int64 value);
int And(int volatile* var, int value);
int64 And(int64 volatile* var, int64 value);
int Xor(int volatile* var, int value);
int64 Xor(int64 volatile* var, int64 value);
int Exchange(int volatile* dest, int value);
int64 Exchange(int64 volatile* dest, int64 value);
int CompareExchange(int volatile* dest, int exchange, int comparand);
int64 CompareExchange(int64 volatile* dest, int64 exchange, int64 comparand);
void* ExchangePointer(void* volatile* dest, void* value);
void* CompareExchangePointer(void* volatile* dest, void* exchange, void* comparand);
int Increment(int volatile* var);
int64 Increment(int64 volatile* var);
int Decrement(int volatile* var);
int64 Decrement(int64 volatile* var);
struct AtomicInt
{
    int Add(int add)
    {
        Threading::Interlocked::Add((volatile int*)&this->value, add);
    }
    int Sub(int sub)
    {
        Threading::Interlocked::Add((volatile int*)&this->value, -sub);
    }
    int Or(int mask)
    {
        Threading::Interlocked::Or((volatile int*)&this->value, mask);
    }
    int And(int mask)
    {
        Threading::Interlocked::And((volatile int*)&this->value, mask);
    }
    int Exchange(int value)
    {
        Threading::Interlocked::Exchange((volatile int*)&this->value, value);
    }
    int CompareExchange(int exchange, int comparand)
    {
        Threading::Interlocked::CompareExchange((volatile int*)&this->value, exchange, comparand);
    }
    int Increment(int incr)
    {
        Threading::Interlocked::Increment((volatile int*)&this->value);
    }
    int Decrement(int decr)
    {
        Threading::Interlocked::Decrement((volatile int*)&this->value);
    }
private:
    volatile int value;
};
struct AtomicInt64
{
    int64 Add(int64 add)
    {
        Threading::Interlocked::Add((volatile int64*)&this->value, add);
    }
    int64 Sub(int64 sub)
    {
        Threading::Interlocked::Add((volatile int64*)&this->value, -sub);
    }
    int64 Or(int64 mask)
    {
        Threading::Interlocked::Or((volatile int64*)&this->value, mask);
    }
    int64 And(int64 mask)
    {
        Threading::Interlocked::And((volatile int64*)&this->value, mask);
    }
    int64 Exchange(int64 value)
    {
        Threading::Interlocked::Exchange((volatile int64*)&this->value, value);
    }
    int64 CompareExchange(int64 exchange, int64 comparand)
    {
        Threading::Interlocked::CompareExchange((volatile int64*)&this->value, exchange, comparand);
    }
    int64 Increment(int64 incr)
    {
        Threading::Interlocked::Increment((volatile int64*)&this->value);
    }
    int64 Decrement(int64 decr)
    {
        Threading::Interlocked::Decrement((volatile int64*)&this->value);
    }
private:
    volatile int64 value;
};
struct AtomicPointer
{
    void* Exchange(void* value)
    {
        return Threading::Interlocked::ExchangePointer((void* volatile*)&this->ptr, value);
    }
    void* CompareExchange(void* exchange, void* comparand)
    {
        return Threading::Interlocked::CompareExchangePointer((void* volatile*)&this->ptr, exchange, comparand);
    }
private:
    volatile void* ptr;
};
}
}