#pragma once
#include "core/types.h"
#include "util/array.h"
namespace Util
{
template<class TYPE> class TrivialArray : public Array<TYPE>
{
public:
    typedef TYPE* Iterator;
    TrivialArray();
    TrivialArray(SizeT initialCapacity, SizeT initialGrow);
    TrivialArray(SizeT initialSize, SizeT initialGrow, const TYPE& initialValue);
    TrivialArray(const TrivialArray<TYPE>& rhs);
    TrivialArray(const Array<TYPE>& rhs);
    TrivialArray(std::initializer_list<TYPE> list);
    ~TrivialArray();
    void operator=(const TrivialArray<TYPE>& rhs);
    void operator=(const Array<TYPE>& rhs);
    void EraseIndex(IndexT index);
    void EraseIndexSwap(IndexT index);
    void Clear();
private:
    void Destroy(TYPE* elm);
    void Copy(const TrivialArray<TYPE>& src);
    void Copy(const Array<TYPE>& src);
    void GrowTo(SizeT newCapacity);
    void Move(IndexT fromIndex, IndexT toIndex);
};
template<class TYPE>
TrivialArray<TYPE>::TrivialArray()
{
    static_assert(std::is_trivial<TYPE>::value, "Используется нетривиальный тип, вместо этого попробуйте Util::Array");
}
template<class TYPE>
TrivialArray<TYPE>::TrivialArray(SizeT _capacity, SizeT _grow) :
    Array<TYPE>(_capacity, _grow)
{
    static_assert(std::is_trivial<TYPE>::value, "Используется нетривиальный тип, вместо этого попробуйте Util::Array");
}
template<class TYPE>
TrivialArray<TYPE>::TrivialArray(SizeT initialSize, SizeT _grow, const TYPE& initialValue) :
    Array<TYPE>(initialSize, _grow, initialValue)
{
    static_assert(std::is_trivial<TYPE>::value, "Используется нетривиальный тип, вместо этого попробуйте Util::Array");
}
template<class TYPE>
TrivialArray<TYPE>::TrivialArray(std::initializer_list<TYPE> list) :
    Array<TYPE>(list)
{
    static_assert(std::is_trivial<TYPE>::value, "Используется нетривиальный тип, вместо этого попробуйте Util::Array");
}
template<class TYPE>
TrivialArray<TYPE>::TrivialArray(const TrivialArray<TYPE>& rhs)
{
    this->elements = 0;
    this->Copy(rhs);
}
template<class TYPE>
TrivialArray<TYPE>::TrivialArray(const Array<TYPE>& rhs)
{
    this->elements = 0;
    this->Copy(rhs);
}
template<class TYPE> void
TrivialArray<TYPE>::Copy(const TrivialArray<TYPE>& src)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 == this->elements);
    #endif
    this->grow = src.grow;
    this->capacity = src.capacity;
    this->size = src.size;
    if (this->capacity > 0)
    {
        this->elements = new TYPE[this->capacity];
        Memory::Copy(src.elements, this->elements, this->size * sizeof(TYPE));
    }
}
template<class TYPE> void
TrivialArray<TYPE>::Copy(const Array<TYPE>& src)
{
#if NEBULA_BOUNDSCHECKS
    n_assert(0 == this->elements);
#endif
    this->grow = src.grow;
    this->capacity = src.capacity;
    this->size = src.size;
    if (this->capacity > 0)
    {
        this->elements = new TYPE[this->capacity];
        Memory::Copy(src.elements, this->elements, this->size * sizeof(TYPE));
    }
}
template<class TYPE> void
TrivialArray<TYPE>::Destroy(TYPE* elm)
{
}
template<class TYPE>
TrivialArray<TYPE>::~TrivialArray()
{
    this->Delete();
}
template<class TYPE> void
TrivialArray<TYPE>::operator=(const TrivialArray<TYPE>& rhs)
{
    if (this != &rhs)
    {
        if ((this->capacity > 0) && (rhs.size <= this->capacity))
        {
            n_assert(0 != this->elements);
            Memory::Copy(rhs.elements, this->elements, rhs.size * sizeof(TYPE));
            this->grow = rhs.grow;
            this->size = rhs.size;
        }
        else
        {
            this->Delete();
            this->Copy(rhs);
        }
    }
}
template<class TYPE> void
TrivialArray<TYPE>::operator=(const Array<TYPE>& rhs)
{
    if (this != &rhs)
    {
        if ((this->capacity > 0) && (rhs.size <= this->capacity))
        {
            n_assert(0 != this->elements);
            Memory::Copy(rhs.elements, this->elements, rhs.size * sizeof(TYPE));
            this->grow = rhs.grow;
            this->size = rhs.size;
        }
        else
        {
            this->Delete();
            this->Copy(rhs);
        }
    }
}
template<class TYPE> void
TrivialArray<TYPE>::GrowTo(SizeT newCapacity)
{
    TYPE* newArray = new TYPE[newCapacity];
    if (this->elements)
    {
        Memory::Copy(this->elements, newArray, this->size * sizeof(TYPE));
        delete[] this->elements;
    }
    this->elements  = newArray;
    this->capacity = newCapacity;
}
template<class TYPE> void
TrivialArray<TYPE>::Move(IndexT fromIndex, IndexT toIndex)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements);
    n_assert(fromIndex < this->size);
    #endif
    if (fromIndex == toIndex)
    {
        return;
    }
    SizeT num = this->size - fromIndex;
    SizeT neededSize = toIndex + num;
    while (neededSize > this->capacity)
    {
        this->Grow();
    }
    if (fromIndex > toIndex)
    {
        IndexT i;
        for (i = 0; i < num; i++)
        {
            this->elements[toIndex + i] = this->elements[fromIndex + i];
        }
    }
    else
    {
        int i;
        for (i = num - 1; i >= 0; --i)
        {
            this->elements[toIndex + i] = this->elements[fromIndex + i];
        }
    }
    this->size = toIndex + num;
}
template<class TYPE> void
TrivialArray<TYPE>::EraseIndex(IndexT index)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (index < this->size));
    #endif
    if (index == (this->size - 1))
    {
        this->size--;
    }
    else
    {
        this->Move(index + 1, index);
    }
}
template<class TYPE> void
TrivialArray<TYPE>::EraseIndexSwap(IndexT index)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (index < this->size));
    #endif
    IndexT lastElementIndex = this->size - 1;
    if (index < lastElementIndex)
    {
        this->elements[index] = this->elements[lastElementIndex];
    }
    this->size--;
}
template<class TYPE> void
TrivialArray<TYPE>::Clear()
{
    this->size = 0;
}
}