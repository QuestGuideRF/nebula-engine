#pragma once
#include "stdneb.h"
#include "threading/event.h"
#include "util/array.h"
namespace Memory
{
struct RingAlloc
{
    uint offset;
    byte* data;
};
template<uint SYNCPOINTS>
class RingAllocator
{
public:
    RingAllocator(const SizeT size);
    ~RingAllocator();
    void Start();
    bool Allocate(const SizeT size, RingAlloc& alloc);
    Threading::Event* End();
private:
    byte* buffer;
    uint size;
    Threading::Event ev[SYNCPOINTS];
    struct Interval
    {
        uint lower, upper;
    };
    enum SyncState
    {
        Signaled,
        Waiting,
        Reset
    };
    Util::FixedArray<Interval> lockedIntervals;
    Util::FixedArray<Threading::Event> events;
    Util::FixedArray<SyncState> states;
    Interval currentInterval;
    Interval freeInterval;
    uint currentAllocation;
    uint nextEvent;
};
template<uint SYNCPOINTS>
inline
RingAllocator<SYNCPOINTS>::RingAllocator(const SizeT size) :
    size(size),
    nextEvent(0)
{
    this->lockedIntervals.Resize(SYNCPOINTS);
    this->events.Resize(SYNCPOINTS);
    this->states.Resize(SYNCPOINTS);
    for (uint i = 0; i < SYNCPOINTS; i++)
    {
        this->states[i] = Reset;
    }
    this->buffer = new byte[size];
    this->currentAllocation = 0;
    this->currentInterval.lower = 0;
    this->currentInterval.upper = 0;
    this->freeInterval.lower = 0;
    this->freeInterval.upper = size;
}
template<uint SYNCPOINTS>
inline
RingAllocator<SYNCPOINTS>::~RingAllocator()
{
    delete[] this->buffer;
}
template<uint SYNCPOINTS>
inline
void RingAllocator<SYNCPOINTS>::Start()
{
    this->currentAllocation = 0;
    this->currentInterval.lower = this->currentInterval.upper = this->freeInterval.lower;
    for (IndexT i = 0; i < this->events.Size(); i++)
    {
        Threading::Event& ev = this->events[i];
        if (ev.Peek() && this->freeInterval.lower)
        {
            this->freeInterval.upper = this->lockedIntervals[i].upper;
            this->states[i] = Signaled;
            ev.Reset();
        }
    }
}
template<uint SYNCPOINTS>
inline bool
RingAllocator<SYNCPOINTS>::Allocate(const SizeT size, RingAlloc& alloc)
{
    n_assert(size <= this->size);
    uint begin = this->currentInterval.upper;
    uint end = begin + size;
    if (end > this->size && this->freeInterval.lower >= this->freeInterval.upper)
    {
        alloc.offset = 0;
        end = this->freeInterval.upper;
    }
    else if (end > this->freeInterval.upper)
    {
        n_warning("RingAllocator перераспределен!\n");
        alloc.data = nullptr;
        alloc.offset = -1;
        return false;
    }
    alloc.data = this->buffer + begin;
    alloc.offset = begin;
    this->currentInterval.upper = end;
    this->currentAllocation += size;
    return true;
}
template<uint SYNCPOINTS>
inline Threading::Event*
RingAllocator<SYNCPOINTS>::End()
{
    Threading::Event* ret = nullptr;
    if (this->currentAllocation > 0)
    {
        this->lockedIntervals[this->nextEvent] = this->currentInterval;
        this->states[this->nextEvent] = Waiting;
        ret = &this->events[this->nextEvent];
        this->nextEvent = (this->nextEvent + 1) % SYNCPOINTS;
        this->freeInterval.lower = this->currentInterval.upper;
    }
    return ret;
}
}