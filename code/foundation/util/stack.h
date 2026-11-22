#pragma once
#include "core/types.h"
#include "util/array.h"
namespace Util
{
template<class TYPE> class Stack
{
public:
    Stack();
    Stack(const Stack<TYPE>& rhs);
    void operator=(const Stack<TYPE>& rhs);
    TYPE& operator[](IndexT index) const;
    bool operator==(const Stack<TYPE>& rhs) const;
    bool operator!=(const Stack<TYPE>& rhs) const;
    SizeT Size() const;
    bool IsEmpty() const;
    void Clear();
    bool Contains(const TYPE& e) const;
    void EraseIndex(const IndexT i);
    void Push(const TYPE& e);
    TYPE& Peek() const;
    TYPE Pop();
private:
    Array<TYPE> stackArray;
};
template<class TYPE>
Stack<TYPE>::Stack()
{
}
template<class TYPE>
Stack<TYPE>::Stack(const Stack<TYPE>& rhs)
{
    this->stackArray = rhs.stackArray;
}
template<class TYPE>
void
Stack<TYPE>::operator=(const Stack<TYPE>& rhs)
{
    this->stackArray = rhs.stackArray;
}
template<class TYPE>
TYPE&
Stack<TYPE>::operator[](IndexT index) const
{
    return this->stackArray[this->stackArray.Size() - 1 - index];
}
template<class TYPE>
bool
Stack<TYPE>::operator==(const Stack<TYPE>& rhs) const
{
    return this->stackArray == rhs.stackArray;
}
template<class TYPE>
bool
Stack<TYPE>::operator!=(const Stack<TYPE>& rhs) const
{
    return this->stackArray != rhs.stackArray;
}
template<class TYPE>
bool
Stack<TYPE>::Contains(const TYPE& e) const
{
    return (InvalidIndex != this->stackArray.FindIndex(e));
}
template<class TYPE>
void
Stack<TYPE>::Clear()
{
    this->stackArray.Clear();
}
template<class TYPE>
SizeT
Stack<TYPE>::Size() const
{
    return this->stackArray.Size();
}
template<class TYPE>
bool
Stack<TYPE>::IsEmpty() const
{
    return this->stackArray.IsEmpty();
}
template<class TYPE>
void
Util::Stack<TYPE>::EraseIndex(const IndexT i)
{
    this->stackArray.EraseIndex(i);
}
template<class TYPE>
void
Stack<TYPE>::Push(const TYPE& e)
{
    this->stackArray.Append(e);
}
template<class TYPE>
TYPE&
Stack<TYPE>::Peek() const
{
    return this->stackArray.Back();
}
template<class TYPE>
TYPE
Stack<TYPE>::Pop()
{
    TYPE e = this->stackArray.Back();
    this->stackArray.EraseIndex(this->stackArray.Size() - 1);
    return e;
}
}