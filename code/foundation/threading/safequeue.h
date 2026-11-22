#pragma once
#include "util/queue.h"
#include "threading/criticalsection.h"
#include "threading/event.h"
namespace Threading
{
template<class TYPE> class SafeQueue
{
public:
    SafeQueue();
    SafeQueue(const SafeQueue<TYPE>& rhs);
    void operator=(const SafeQueue<TYPE>& rhs);
    void SetSignalOnEnqueueEnabled(bool b);
    bool IsSignalOnEnqueueEnabled() const;
    SizeT Size() const;
    bool IsEmpty() const;
    void Clear();
    void Enqueue(const TYPE& e);
    void Enqueue(TYPE&& e);
    void EnqueueArray(const Util::Array<TYPE>& a);
    TYPE Dequeue();
    void DequeueAll(Util::Array<TYPE>& outArray);
    template<int STACK_SIZE>
    void DequeueAll(Util::Array<TYPE, STACK_SIZE>& outArray);
    TYPE Peek() const;
    void Wait();
    void WaitTimeout(int ms);
    void Signal();
    void EraseMatchingElements(const TYPE& e);
protected:
    CriticalSection criticalSection;
    Event enqueueEvent;
    bool signalOnEnqueueEnabled;
    Util::Queue<TYPE> queue;
};
template<class TYPE>
SafeQueue<TYPE>::SafeQueue() :
    signalOnEnqueueEnabled(true)
{
}
template<class TYPE>
SafeQueue<TYPE>::SafeQueue(const SafeQueue<TYPE>& rhs)
{
    this->criticalSection.Enter();
    this->queue = rhs.queue;
    this->signalOnEnqueueEnabled = rhs.signalOnEnqueueEnabled;
    this->criticalSection.Leave();
}
template<class TYPE> void
SafeQueue<TYPE>::operator=(const SafeQueue<TYPE>& rhs)
{
    this->criticalSection.Enter();
    this->queue = rhs.queue;
    this->signalOnEnqueueEnabled = rhs.signalOnEnqueueEnabled;
    this->criticalSection.Leave();
}
template<class TYPE> void
SafeQueue<TYPE>::SetSignalOnEnqueueEnabled(bool b)
{
    this->criticalSection.Enter();
    this->signalOnEnqueueEnabled = b;
    this->criticalSection.Leave();
}
template<class TYPE> void
SafeQueue<TYPE>::Clear()
{
    this->criticalSection.Enter();
    this->queue.Clear();
    this->criticalSection.Leave();
}
template<class TYPE> SizeT
SafeQueue<TYPE>::Size() const
{
    return this->queue.Size();
}
template<class TYPE> bool
SafeQueue<TYPE>::IsEmpty() const
{
    this->criticalSection.Enter();
    bool isEmpty = this->queue.IsEmpty();
    this->criticalSection.Leave();
    return isEmpty;
}
template<class TYPE> void
SafeQueue<TYPE>::Enqueue(const TYPE& e)
{
    this->criticalSection.Enter();
    this->queue.Enqueue(e);
    this->criticalSection.Leave();
    if (this->signalOnEnqueueEnabled)
    {
        this->enqueueEvent.Signal();
    }
}
template<class TYPE> void
SafeQueue<TYPE>::Enqueue(TYPE&& e)
{
    this->criticalSection.Enter();
    this->queue.Enqueue(std::move(e));
    this->criticalSection.Leave();
    if (this->signalOnEnqueueEnabled)
    {
        this->enqueueEvent.Signal();
    }
}
template<class TYPE> void
SafeQueue<TYPE>::EnqueueArray(const Util::Array<TYPE>& a)
{
    this->criticalSection.Enter();
    for (auto &i : a)
    {
        this->queue.Enqueue(i);
    }
    this->criticalSection.Leave();
    if (this->signalOnEnqueueEnabled)
    {
        this->enqueueEvent.Signal();
    }
}
template<class TYPE> TYPE
SafeQueue<TYPE>::Dequeue()
{
    this->criticalSection.Enter();
    TYPE e = this->queue.Dequeue();
    this->criticalSection.Leave();
    return e;
}
template<class TYPE> void
SafeQueue<TYPE>::DequeueAll(Util::Array<TYPE>& outArray)
{
    this->criticalSection.Enter();
#if NEBULA_ENABLE_PERFORMANCE_WARNINGS
    n_warn_fmt(outArray.Capacity() >= this->queue.Size(), "SafeQueue::DequeueAll(): (PERFORMANCE) Выходной массив слишком мал (%d), требуется (%d), массив должен вырасти.\n", outArray.Capacity(), this->queue.Size());
#endif
    outArray.Clear();
    for (IndexT i = 0; i < this->queue.Size(); i++)
    {
        outArray.Append(std::move(this->queue[i]));
    }
    this->queue.Clear();
    this->criticalSection.Leave();
}
template<class TYPE>
template<int STACK_SIZE>
void
SafeQueue<TYPE>::DequeueAll(Util::Array<TYPE, STACK_SIZE>& outArray)
{
    this->criticalSection.Enter();
#if NEBULA_ENABLE_PERFORMANCE_WARNINGS
    n_warn_fmt(outArray.Capacity() >= this->queue.Size(), "SafeQueue::DequeueAll(): (PERFORMANCE) Выходной массив слишком мал (%d), требуется (%d), массив должен вырасти.\n", outArray.Capacity(), this->queue.Size());
#endif
    outArray.Clear();
    for (IndexT i = 0; i < this->queue.Size(); i++)
    {
        outArray.Append(std::move(this->queue[i]));
    }
    this->queue.Clear();
    this->criticalSection.Leave();
}
template<class TYPE> TYPE
SafeQueue<TYPE>::Peek() const
{
    this->criticalSection.Enter();
    TYPE e = this->queue.Peek();
    this->criticalSection.Leave();
    return e;
}
template<class TYPE> void
SafeQueue<TYPE>::Wait()
{
    if (this->signalOnEnqueueEnabled && this->IsEmpty())
    {
        this->enqueueEvent.Wait();
    }
}
template<class TYPE> void
SafeQueue<TYPE>::WaitTimeout(int ms)
{
    if (this->signalOnEnqueueEnabled && this->IsEmpty())
    {
        this->enqueueEvent.WaitTimeout(ms);
    }
}
template<class TYPE> void
SafeQueue<TYPE>::Signal()
{
    this->enqueueEvent.Signal();
}
template<class TYPE> void
SafeQueue<TYPE>::EraseMatchingElements(const TYPE& e)
{
    this->criticalSection.Enter();
    IndexT i;
    for (i = this->queue.Size() - 1; i >= 0; i--)
    {
        if (e == this->queue[i])
        {
            this->queue.EraseIndex(i);
        }
    }
    this->criticalSection.Leave();
}
}