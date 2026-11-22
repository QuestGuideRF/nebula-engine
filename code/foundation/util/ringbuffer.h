#pragma once
#include "core/types.h"
#include "util/array.h"
namespace Util
{
template <class TYPE> class RingBuffer
{
public:
    RingBuffer();
    RingBuffer(SizeT capacity);
    RingBuffer(const RingBuffer<TYPE>& rhs);
    RingBuffer(RingBuffer<TYPE>&& rhs);
    ~RingBuffer();
    void operator=(const RingBuffer<TYPE>& rhs);
    void operator=(RingBuffer<TYPE>&& rhs);
    TYPE& operator[](IndexT index) const;
    void SetCapacity(SizeT newCapacity);
    SizeT Capacity() const;
    SizeT Size() const;
    void Add(const TYPE& elm);
    bool IsEmpty() const;
    void Reset();
    TYPE& Front() const;
    TYPE& Back() const;
    Array<TYPE> AsArray() const;
    TYPE* GetBuffer();
    const TYPE* GetBuffer() const;
private:
    void Allocate(SizeT capacity);
    void Delete();
    void Copy(const RingBuffer<TYPE>& src);
    SizeT capacity;
    SizeT size;
    IndexT baseIndex;
    IndexT headIndex;
    TYPE* elements;
};
template<class TYPE> void
RingBuffer<TYPE>::Allocate(SizeT c)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 == this->elements);
    n_assert(c > 0);
    #endif
    this->capacity = c;
    this->size = 0;
    this->baseIndex = 0;
    this->headIndex = 0;
    this->elements = new TYPE[c];
}
template<class TYPE> void
RingBuffer<TYPE>::Delete()
{
    this->capacity = 0;
    this->size = 0;
    this->baseIndex = 0;
    this->headIndex = 0;
    if (0 != this->elements)
    {
        delete[] this->elements;
        this->elements = 0;
    }
}
template<class TYPE> void
RingBuffer<TYPE>::Copy(const RingBuffer<TYPE>& rhs)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 == this->elements);
    #endif
    this->Allocate(rhs.capacity);
    this->size = rhs.size;
    this->baseIndex = rhs.baseIndex;
    this->headIndex = rhs.headIndex;
    IndexT i;
    for (i = 0; i < rhs.Size(); i++)
    {
        (*this)[i] = rhs[i];
    }
}
template<class TYPE>
RingBuffer<TYPE>::RingBuffer() :
    capacity(0),
    size(0),
    baseIndex(0),
    headIndex(0),
    elements(0)
{
}
template<class TYPE>
RingBuffer<TYPE>::RingBuffer(SizeT c) :
    elements(0)
{
    this->Allocate(c);
}
template<class TYPE>
RingBuffer<TYPE>::RingBuffer(const RingBuffer<TYPE>& rhs) :
    elements(0)
{
    if (rhs.size > 0)
        this->Copy(rhs);
}
template<class TYPE>
RingBuffer<TYPE>::RingBuffer(RingBuffer<TYPE>&& rhs)
{
    this->size = rhs.size;
    this->capacity = rhs.capacity;
    this->headIndex = rhs.headIndex;
    this->baseIndex = rhs.baseIndex;
    this->elements = rhs.elements;
    rhs.size = 0;
    rhs.capacity = 0;
    rhs.headIndex = 0;
    rhs.baseIndex = 0;
    rhs.elements = nullptr;
}
template<class TYPE>
RingBuffer<TYPE>::~RingBuffer()
{
    this->Delete();
}
template<class TYPE> void
RingBuffer<TYPE>::operator=(const RingBuffer<TYPE>& rhs)
{
    this->Delete();
    if (rhs.capacity > 0)
        this->Copy(rhs);
}
template<class TYPE> void
RingBuffer<TYPE>::operator=(RingBuffer<TYPE>&& rhs)
{
    this->size = rhs.size;
    this->capacity = rhs.capacity;
    this->headIndex = rhs.headIndex;
    this->baseIndex = rhs.baseIndex;
    this->elements = rhs.elements;
    rhs.size = 0;
    rhs.capacity = 0;
    rhs.headIndex = 0;
    rhs.baseIndex = 0;
    rhs.elements = nullptr;
}
template<class TYPE> TYPE&
RingBuffer<TYPE>::operator[](IndexT index) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (index < this->size));
    #endif
    IndexT absIndex = index + this->baseIndex;
    if (absIndex >= this->capacity)
    {
        absIndex -= this->capacity;
    }
    return this->elements[absIndex];
}
template<class TYPE> TYPE&
RingBuffer<TYPE>::Front() const
{
    return (*this)[0];
}
template<class TYPE> TYPE&
RingBuffer<TYPE>::Back() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->size > 0);
    #endif
    return (*this)[this->size - 1];
}
template<class TYPE> bool
RingBuffer<TYPE>::IsEmpty() const
{
    return (0 == this->size);
}
template<class TYPE> void
RingBuffer<TYPE>::Reset()
{
    this->size = 0;
    this->baseIndex = 0;
    this->headIndex = 0;
}
template<class TYPE> void
RingBuffer<TYPE>::SetCapacity(SizeT newCapacity)
{
    this->Delete();
    this->Allocate(newCapacity);
}
template<class TYPE> SizeT
RingBuffer<TYPE>::Capacity() const
{
    return this->capacity;
}
template<class TYPE> SizeT
RingBuffer<TYPE>::Size() const
{
    return this->size;
}
template<class TYPE> void
RingBuffer<TYPE>::Add(const TYPE& elm)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != this->elements);
    #endif
    this->elements[this->headIndex++] = elm;
    if (this->headIndex >= this->capacity)
    {
        this->headIndex = 0;
    }
    if (this->size == this->capacity)
    {
        this->baseIndex++;
        if (this->baseIndex >= this->capacity)
        {
            this->baseIndex = 0;
        }
    }
    else
    {
        this->size++;
    }
}
template<class TYPE> Array<TYPE>
RingBuffer<TYPE>::AsArray() const
{
    Array<TYPE> result(this->size, 0);
    IndexT i;
    for (i = 0; i < this->size; i++)
    {
        result.Append((*this)[i]);
    }
    return result;
}
template<class TYPE>
TYPE*
RingBuffer<TYPE>::GetBuffer()
{
    n_assert(this->elements);
    return this->elements;
}
template<class TYPE>
const TYPE*
RingBuffer<TYPE>::GetBuffer() const
{
    n_assert(this->elements);
    return this->elements;
}
}