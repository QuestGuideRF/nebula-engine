#pragma once
#include "core/types.h"
#include "util/array.h"
namespace Util
{
template<class TYPE> class ArrayQueue
{
public:
    ArrayQueue();
    ArrayQueue(const ArrayQueue<TYPE>& rhs);
    ArrayQueue(ArrayQueue<TYPE>&& rhs);
    ArrayQueue(const Array<TYPE>& rhs);
    void operator=(const ArrayQueue<TYPE>& rhs);
    void operator=(ArrayQueue<TYPE>&& rhs);
    TYPE& operator[](IndexT index) const;
    bool operator==(const ArrayQueue<TYPE>& rhs) const;
    bool operator!=(const ArrayQueue<TYPE>& rhs) const;
    void Reserve(SizeT num);
    SizeT Size() const;
    bool IsEmpty() const;
    void Clear();
    bool Contains(const TYPE& e) const;
    void EraseIndex(const IndexT i);
    void Enqueue(const TYPE& e);
    TYPE Dequeue();
    TYPE& Peek() const;
protected:
    Array<TYPE> queueArray;
};
template<class TYPE>
ArrayQueue<TYPE>::ArrayQueue()
{
}
template<class TYPE>
ArrayQueue<TYPE>::ArrayQueue(const ArrayQueue<TYPE>& rhs)
{
    this->queueArray = rhs.queueArray;
}
template<class TYPE>
ArrayQueue<TYPE>::ArrayQueue(ArrayQueue<TYPE>&& rhs):
    queueArray(std::move(rhs.queueArray))
{
}
template<class TYPE>
Util::ArrayQueue<TYPE>::ArrayQueue(const Array<TYPE>& rhs)
{
    this->queueArray = rhs;
}
template<class TYPE>
void
ArrayQueue<TYPE>::operator=(const ArrayQueue<TYPE>& rhs)
{
    this->queueArray = rhs.queueArray;
}
template<class TYPE>
void
ArrayQueue<TYPE>::operator=(ArrayQueue<TYPE>&& rhs)
{
    this->queueArray = std::move(rhs.queueArray);
}
template<class TYPE>
TYPE&
ArrayQueue<TYPE>::operator[](IndexT index) const
{
    return this->queueArray[index];
}
template<class TYPE>
bool
ArrayQueue<TYPE>::operator==(const ArrayQueue<TYPE>& rhs) const
{
    return this->queueArray == rhs.queueArray;
}
template<class TYPE>
bool
ArrayQueue<TYPE>::operator!=(const ArrayQueue<TYPE>& rhs) const
{
    return this->queueArray != rhs.queueArray;
}
template<class TYPE>
bool
ArrayQueue<TYPE>::Contains(const TYPE& e) const
{
    return (InvalidIndex != this->queueArray.FindIndex(e));
}
template<class TYPE>
void
Util::ArrayQueue<TYPE>::EraseIndex(const IndexT i)
{
    this->queueArray.EraseIndex(i);
}
template<class TYPE>
void
ArrayQueue<TYPE>::Clear()
{
    this->queueArray.Clear();
}
template<class TYPE>
void
ArrayQueue<TYPE>::Reserve(SizeT num)
{
    this->queueArray.Reserve(num);
}
template<class TYPE>
SizeT
ArrayQueue<TYPE>::Size() const
{
    return this->queueArray.Size();
}
template<class TYPE>
bool
ArrayQueue<TYPE>::IsEmpty() const
{
    return this->queueArray.IsEmpty();
}
template<class TYPE>
void
ArrayQueue<TYPE>::Enqueue(const TYPE& e)
{
    this->queueArray.Append(e);
}
template<class TYPE>
TYPE
ArrayQueue<TYPE>::Dequeue()
{
    TYPE e = this->queueArray.Front();
    this->queueArray.EraseIndex(0);
    return e;
}
template<class TYPE>
TYPE&
ArrayQueue<TYPE>::Peek() const
{
    return this->queueArray.Front();
}
}