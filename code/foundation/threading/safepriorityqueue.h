#pragma once
#include "util/queue.h"
#include "threading/criticalsection.h"
#include "threading/event.h"
namespace Threading
{
template<class PRITYPE, class TYPE> class SafePriorityQueue : protected Util::Queue<Util::KeyValuePair<PRITYPE, TYPE> >
{
public:
    SafePriorityQueue();
    SafePriorityQueue(const SafePriorityQueue<PRITYPE,TYPE>& rhs);
    void operator=(const SafePriorityQueue<PRITYPE,TYPE>& rhs);
    SizeT Size() const;
    bool IsEmpty() const;
    void Clear();
    void Insert(PRITYPE pri, const TYPE& e);
    void EraseMatchingElements(const TYPE& e);
    TYPE Dequeue();
    TYPE Peek() const;
    void Wait();
    void Signal();
protected:
    CriticalSection criticalSection;
    Event enqueueEvent;
};
template<class PRITYPE, class TYPE>
SafePriorityQueue<PRITYPE,TYPE>::SafePriorityQueue()
{
}
template<class PRITYPE, class TYPE>
SafePriorityQueue<PRITYPE,TYPE>::SafePriorityQueue(const SafePriorityQueue<PRITYPE,TYPE>& rhs)
{
    this->criticalSection.Enter();
    this->queueArray = rhs.queueArray;
    this->criticalSection.Leave();
}
template<class PRITYPE, class TYPE> void
SafePriorityQueue<PRITYPE,TYPE>::operator=(const SafePriorityQueue<PRITYPE,TYPE>& rhs)
{
    this->criticalSection.Enter();
    this->queueArray = rhs.queueArray;
    this->criticalSection.Leave();
}
template<class PRITYPE, class TYPE> void
SafePriorityQueue<PRITYPE,TYPE>::Clear()
{
    this->criticalSection.Enter();
    this->queueArray.Clear();
    this->criticalSection.Leave();
}
template<class PRITYPE, class TYPE> SizeT
SafePriorityQueue<PRITYPE,TYPE>::Size() const
{
    return this->queueArray.Size();
}
template<class PRITYPE, class TYPE> bool
SafePriorityQueue<PRITYPE,TYPE>::IsEmpty() const
{
    return this->queueArray.IsEmpty();
}
template<class PRITYPE, class TYPE> void
SafePriorityQueue<PRITYPE,TYPE>::Insert(PRITYPE pri, const TYPE& e)
{
    Util::KeyValuePair<PRITYPE, TYPE> kvp(pri, e);
    this->criticalSection.Enter();
    this->queueArray.InsertSorted(kvp);
    this->criticalSection.Leave();
    this->enqueueEvent.Signal();
}
template<class PRITYPE, class TYPE> void
SafePriorityQueue<PRITYPE,TYPE>::EraseMatchingElements(const TYPE& e)
{
    this->criticalSection.Enter();
    for (auto iter = this->queueArray.Begin(); iter != this->queueArray.End();)
    {
        if ((*iter).Value() == e)
        {
            iter = this->queueArray.Erase(iter);
        }
        else
        {
            iter++;
        }
    }
    this->criticalSection.Leave();
}
template<class PRITYPE, class TYPE> TYPE
SafePriorityQueue<PRITYPE,TYPE>::Dequeue()
{
    this->criticalSection.Enter();
    TYPE value = this->queueArray.Front().Value();
    this->queueArray.EraseIndex(0);
    this->criticalSection.Leave();
    return value;
}
template<class PRITYPE, class TYPE> TYPE
SafePriorityQueue<PRITYPE,TYPE>::Peek() const
{
    this->criticalSection.Enter();
    TYPE value = this->queueArray.Front().Value();
    this->criticalSection.Leave();
    return value;
}
template<class PRITYPE, class TYPE> void
SafePriorityQueue<PRITYPE,TYPE>::Wait()
{
    if (this->queueArray.Size() == 0)
    {
        this->enqueueEvent.Wait();
    }
}
template<class PRITYPE, class TYPE> void
SafePriorityQueue<PRITYPE,TYPE>::Signal()
{
    this->enqueueEvent.Signal();
}
}