#pragma once
#include "core/types.h"
#include "math/scalar.h"
#include "util/round.h"
#include <type_traits>
#if (__cplusplus >= 201703L ) || (_MSVC_LANG >= 201703L)
#define IFCONSTEXPR if constexpr
#else
#define IFCONSTEXPR if
#endif
namespace Util
{
template<class TYPE> class Queue
{
public:
    Queue();
    ~Queue();
    Queue(const Queue<TYPE>& rhs);
    Queue(Queue<TYPE>&& rhs);
    void operator=(const Queue<TYPE>& rhs);
    void operator=(Queue<TYPE>&& rhs);
    TYPE& operator[](IndexT index) const;
    bool operator==(const Queue<TYPE>& rhs) const;
    bool operator!=(const Queue<TYPE>& rhs) const;
    void Reserve(SizeT num);
    void Grow();
    SizeT Size() const;
    SizeT Capacity() const;
    bool IsEmpty() const;
    void Clear();
    bool Contains(const TYPE& e) const;
    void EraseIndex(const IndexT i);
    void Enqueue(const TYPE& e);
    void Enqueue(TYPE&& e);
    TYPE Dequeue();
    TYPE& Peek() const;
protected:
    template<typename X>
    __forceinline
    typename std::enable_if<std::is_trivially_destructible<X>::value == false>::type
    DestroyElement(IndexT idx)
    {
        this->data[idx].~X();
    }
    template<typename X>
    __forceinline
    typename std::enable_if<std::is_trivially_destructible<X>::value == true>::type
    DestroyElement(IndexT idx)
    {
    }
    template<typename X>
    __forceinline
        typename std::enable_if<std::is_trivially_destructible<X>::value == false>::type
        ClearAll()
    {
        for (IndexT i = 0; i < this->size; i++)
        {
            this->data[this->MapIndex(i)].~X();
        }
    }
    template<typename X>
    __forceinline
        typename std::enable_if<std::is_trivially_destructible<X>::value == true>::type
        ClearAll()
    {
    }
    IndexT MapIndex(IndexT index) const;
    TYPE * data;
    static const SizeT MinGrowSize = 16;
    static const SizeT MaxGrowSize = 65536;
    SizeT grow;
    SizeT start;
    SizeT size;
    SizeT capacity;
};
template<class TYPE>
Queue<TYPE>::Queue():
    data(nullptr),
    grow(16),
    start(0),
    size(0),
    capacity(0)
{
}
template<class TYPE>
Queue<TYPE>::~Queue()
{
    if (this->data)
    {
        this->Clear();
        delete[] this->data;
    }
    this->data = nullptr;
}
template<class TYPE>
Queue<TYPE>::Queue(const Queue<TYPE>& rhs):
    data(nullptr),
    capacity(0)
{
    this->Reserve(rhs.size);
    for (IndexT i = 0; i < rhs.size; i++)
    {
        this->data[i] = rhs[i];
    }
    this->size = rhs.size;
    this->grow = rhs.grow;
    this->start = 0;
}
template<class TYPE>
Queue<TYPE>::Queue(Queue<TYPE>&& rhs) :
    data(rhs.data),
    capacity(rhs.capacity),
    size(rhs.size),
    grow(rhs.grow),
    start(rhs.start)
{
    rhs.data = nullptr;
    rhs.capacity = 0;
    rhs.size = 0;
}
template<class TYPE>
void
Queue<TYPE>::operator=(const Queue<TYPE>& rhs)
{
    if (this != &rhs)
    {
        this->Reserve(rhs.size);
        for (IndexT i = 0; i < rhs.size; i++)
        {
            this->data[i] = rhs[i];
        }
        this->size = rhs.size;
        this->grow = rhs.grow;
        this->start = 0;
    }
}
template<class TYPE>
void
Queue<TYPE>::operator=(Queue<TYPE>&& rhs)
{
    if (this != &rhs)
    {
        this->data = rhs.data;
        this->size = rhs.size;
        this->grow = rhs.grow;
        this->capacity = rhs.capacity;
        this->start = rhs.start;
        rhs.data = nullptr;
        rhs.size = 0;
    }
}
template<class TYPE>
__forceinline
TYPE&
Queue<TYPE>::operator[](IndexT index) const
{
    n_assert(index < this->size);
    return this->data[MapIndex(index)];
}
template<class TYPE>
bool
Queue<TYPE>::operator==(const Queue<TYPE>& rhs) const
{
    if(this->size != rhs.size) return false;
    for(IndexT i= 0; i < this->size; i++)
    {
        if(!((*this)[i] == rhs[i] ))
        {
            return false;
        }
    }
    return true;
}
template<class TYPE>
bool
Queue<TYPE>::operator!=(const Queue<TYPE>& rhs) const
{
    return ! this->operator==(rhs);
}
template<class TYPE>
bool
Queue<TYPE>::Contains(const TYPE& e) const
{
    for(IndexT i = 0 ; i<this->size; i++)
    {
        if (this->operator[](i) == e)
            return true;
    }
    return false;
}
template<class TYPE>
void
Util::Queue<TYPE>::EraseIndex(const IndexT i)
{
    n_assert(i < this->size);
    if (i == 0)
    {
        this->Dequeue();
    }
    else
    {
        IndexT idx = this->MapIndex(i);
        this->DestroyElement<TYPE>(idx);
        if (idx < this->start)
        {
            for (IndexT j = 0; j < this->size - i; ++j)
            {
                this->data[idx] = this->data[idx+1];
                idx++;
            }
        }
        else
        {
            for (IndexT j = 0; j < i; j++)
            {
                this->data[idx] = this->data[idx-1];
                --idx;
            }
            this->start = this->MapIndex(1);
        }
        --this->size;
    }
}
template<class TYPE>
void
Queue<TYPE>::Clear()
{
    this->ClearAll<TYPE>();
    this->size = 0;
    this->start = 0;
}
template<class TYPE>
void
Queue<TYPE>::Reserve(SizeT num)
{
    if (num > this->capacity)
    {
        num = num<64? num: Util::Round::RoundUp(num, 64);
        TYPE * newdata = new TYPE[num];
        if (this->capacity > 0)
        {
            IFCONSTEXPR(std::is_trivially_copyable_v<TYPE>)
            {
                if (this->size > 0)
                {
                    SizeT upper = this->capacity - this->start;
                    SizeT lower = this->size - (this->capacity - this->start);
                    if (lower < 0)
                    {
                        Memory::Copy(&this->data[this->start], newdata, this->size * sizeof(TYPE));
                    }
                    else
                    {
                        Memory::Copy(&this->data[this->start], newdata, upper * sizeof(TYPE));
                        Memory::Copy(this->data, &newdata[upper], lower * sizeof(TYPE));
                    }
                }
                if (this->data != nullptr)
                {
                    delete[] this->data;
                }
            }
            else
            {
                for (IndexT i = 0; i < this->size; i++)
                {
                    IndexT idx = this->MapIndex(i);
                    newdata[i] = std::move(this->data[idx]);
                    this->DestroyElement<TYPE>(idx);
                }
                if (this->data != nullptr)
                {
                    delete[] this->data;
                }
            }
        }
        this->data = newdata;
        this->capacity = num;
        this->start = 0;
    }
}
template<class TYPE> void
Queue<TYPE>::Grow()
{
    SizeT growToSize;
    if (0 == this->capacity)
    {
        growToSize = this->grow;
    }
    else
    {
        SizeT growBy = this->capacity >> 1;
        if (growBy == 0)
        {
            growBy = MinGrowSize;
        }
        else if (growBy > MaxGrowSize)
        {
            growBy = MaxGrowSize;
        }
        growToSize = this->capacity + growBy;
    }
    this->Reserve(growToSize);
}
template<class TYPE>
__forceinline
SizeT
Queue<TYPE>::Size() const
{
    return this->size;
}
template<class TYPE>
__forceinline
SizeT
Queue<TYPE>::Capacity() const
{
    return this->capacity;
}
template<class TYPE>
__forceinline
bool
Queue<TYPE>::IsEmpty() const
{
    return this->size == 0;
}
template<class TYPE>
__forceinline
void
Queue<TYPE>::Enqueue(const TYPE& e)
{
    if (this->size == this->capacity)
    {
        this->Grow();
    }
    this->data[this->MapIndex(this->size++)] = e;
}
template<class TYPE>
__forceinline
void
Queue<TYPE>::Enqueue(TYPE&& e)
{
    if (this->size == this->capacity)
    {
        this->Grow();
    }
    this->data[this->MapIndex(this->size++)]= std::move(e);
}
template<class TYPE>
__forceinline
TYPE
Queue<TYPE>::Dequeue()
{
    n_assert(this->size > 0);
    TYPE t = this->data[this->start];
    #if __cplusplus > 201703L
    if constexpr (!std::is_nothrow_destructible_v<TYPE>)
    {
        (&(this->data[this->start]))->~TYPE();
    }
    #else
    this->DestroyElement<TYPE>(this->start);
    #endif
    this->start = this->MapIndex(1);
    --this->size;
    return t;
}
template<class TYPE>
__forceinline
TYPE&
Queue<TYPE>::Peek() const
{
    n_assert(this->size > 0);
    return this->data[this->start];
}
template<class TYPE>
__forceinline
IndexT
Queue<TYPE>::MapIndex(IndexT idx) const
{
    idx = (idx + this->start) % this->capacity;
    if (idx > 0)
    {
        return idx;
    }
    else
    {
        return (idx + this->capacity) % this->capacity;
    }
}
}