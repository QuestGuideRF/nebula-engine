#pragma once
#include "core/types.h"
#include "system/systeminfo.h"
#include "math/scalar.h"
#include <type_traits>
namespace Util
{
template<class TYPE, int STACK_SIZE>
struct _smallvector
{
    TYPE* data() { return stackElements; }
private:
    TYPE stackElements[STACK_SIZE];
};
template<class TYPE>
struct _smallvector<TYPE, 0>
{
    TYPE* data() { return nullptr; }
};
template<class TYPE, int SMALL_VECTOR_SIZE = 0> class Array
{
public:
    typedef TYPE* Iterator;
    typedef const TYPE* ConstIterator;
    using ArrayT = Array<TYPE, SMALL_VECTOR_SIZE>;
    Array();
    Array(SizeT initialCapacity, SizeT initialGrow);
    Array(SizeT initialSize, SizeT initialGrow, const TYPE& initialValue);
    Array(const ArrayT& rhs);
    Array(ArrayT&& rhs) noexcept;
    Array(std::initializer_list<TYPE> list);
    Array(std::nullptr_t);
    Array(const TYPE* const buf, SizeT num);
    ~Array();
    void operator=(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs);
    void operator=(Array<TYPE, SMALL_VECTOR_SIZE>&& rhs) noexcept;
    TYPE& operator[](IndexT index) const;
    TYPE& operator[](IndexT index);
    bool operator==(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs) const;
    bool operator!=(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs) const;
    template<typename T> T As() const;
    TYPE& Get(IndexT index) const;
    template <typename ...ELEM_TYPE>
    void Append(const TYPE& first, const ELEM_TYPE&... elements);
    void Append(const TYPE& elm);
    void Append(TYPE&& elm);
    void AppendArray(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs);
    void AppendArray(const TYPE* arr, const SizeT count);
    TYPE& Emplace();
    TYPE* EmplaceArray(const SizeT count);
    void Reserve(SizeT num);
    const SizeT Size() const;
    const SizeT ByteSize() const;
    const SizeT Capacity() const;
    TYPE& Front() const;
    TYPE& Back() const;
    bool IsEmpty() const;
    bool IsValidIndex(IndexT index) const;
    void EraseIndex(IndexT index);
    Iterator Erase(Iterator iter);
    void EraseIndexSwap(IndexT index);
    Iterator EraseSwap(Iterator iter);
    void EraseRange(IndexT start, IndexT end);
    void EraseBack();
    void EraseFront();
    TYPE PopFront();
    TYPE PopBack();
    void Insert(IndexT index, const TYPE& elm);
    IndexT InsertSorted(const TYPE& elm);
    IndexT InsertAtEndOfIdenticalRange(IndexT startIndex, const TYPE& elm);
    bool IsSorted() const;
    void Clear();
    void Reset();
    void Free();
    Iterator Begin() const;
    ConstIterator ConstBegin() const;
    Iterator End() const;
    ConstIterator ConstEnd() const;
    Iterator Find(const TYPE& elm, const IndexT start = 0) const;
    IndexT FindIndex(const TYPE& elm, const IndexT start = 0) const;
    template <typename KEYTYPE> IndexT FindIndex(typename std::enable_if<true, const KEYTYPE&>::type elm, const IndexT start = 0) const;
    void Fill(IndexT first, SizeT num, const TYPE& elm);
    void Realloc(SizeT capacity, SizeT grow);
    ArrayT Difference(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs);
    void Sort();
    void QuickSort();
    void SortWithFunc(bool (*func)(const TYPE& lhs, const TYPE& rhs));
    void QuickSortWithFunc(int (*func)(const void* lhs, const void* rhs));
    IndexT BinarySearchIndex(const TYPE& elm) const;
    template <typename KEYTYPE> IndexT BinarySearchIndex(typename std::enable_if<true, const KEYTYPE&>::type elm) const;
    void Resize(SizeT num);
    template <typename ...ARGS> void Resize(SizeT num, ARGS... args);
    void Extend(SizeT num);
    void Fit();
    constexpr SizeT TypeSize() const;
    Iterator begin() const;
    Iterator end() const;
    size_t size() const;
    void resize(size_t size);
    void clear() noexcept;
    void push_back(const TYPE& item);
    void Grow();
protected:
    template<class T, bool S>
    friend class FixedArray;
    void Destroy(TYPE* elm);
    void Copy(const Array<TYPE, SMALL_VECTOR_SIZE>& src);
    void Delete();
    void GrowTo(SizeT newCapacity);
    void Move(IndexT fromIndex, IndexT toIndex);
    void DestroyRange(IndexT fromIndex, IndexT toIndex);
    void CopyRange(TYPE* to, TYPE* from, SizeT num);
    void MoveRange(TYPE* to, TYPE* from, SizeT num);
    static const SizeT MinGrowSize = 16;
    static const SizeT MaxGrowSize = 65536;
    SizeT grow;
    SizeT capacity;
    SizeT count;
    TYPE* elements;
    _smallvector<TYPE, SMALL_VECTOR_SIZE> stackElements;
};
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array() :
    grow(16),
    capacity(SMALL_VECTOR_SIZE),
    count(0),
    elements(this->stackElements.data())
{
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array(SizeT _capacity, SizeT _grow) :
    grow(_grow),
    capacity(SMALL_VECTOR_SIZE),
    count(0),
    elements(this->stackElements.data())
{
    if (0 == this->grow)
    {
        this->grow = 16;
    }
    this->GrowTo(_capacity);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array(SizeT initialSize, SizeT _grow, const TYPE& initialValue) :
    grow(_grow),
    capacity(SMALL_VECTOR_SIZE),
    count(0),
    elements(stackElements.data())
{
    if (0 == this->grow)
    {
        this->grow = 16;
    }
    this->GrowTo(initialSize);
    this->count = initialSize;
    IndexT i;
    for (i = 0; i < initialSize; i++)
    {
        this->elements[i] = initialValue;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array(const TYPE* const buf, SizeT num) :
    grow(16),
    capacity(SMALL_VECTOR_SIZE),
    count(0),
    elements(stackElements.data())
{
    static_assert(std::is_trivially_copyable<TYPE>::value, "TYPE не является тривиальным копированием; Util::Array не может быть создан из указателя TYPE.");
    this->GrowTo(num);
    this->count = num;
    const SizeT bytes = num * sizeof(TYPE);
    Memory::Copy(buf, this->elements, bytes);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array(std::initializer_list<TYPE> list) :
    grow(16),
    capacity(SMALL_VECTOR_SIZE),
    count(0),
    elements(stackElements.data())
{
    this->GrowTo((SizeT)list.size());
    this->count = (SizeT)list.size();
    IndexT i;
    for (i = 0; i < this->count; i++)
    {
        this->elements[i] = list.begin()[i];
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array(std::nullptr_t) :
    grow(16),
    capacity(SMALL_VECTOR_SIZE),
    count(0),
    elements(stackElements.data())
{
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs) :
    grow(16),
    capacity(SMALL_VECTOR_SIZE),
    count(0),
    elements(this->stackElements.data())
{
    this->Copy(rhs);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Array(Array<TYPE, SMALL_VECTOR_SIZE>&& rhs) noexcept :
    grow(rhs.grow),
    capacity(rhs.capacity),
    count(rhs.count),
    elements(this->stackElements.data())
{
    if (rhs.capacity <= SMALL_VECTOR_SIZE)
    {
        for (IndexT i = 0; i < rhs.capacity; i++)
            this->elements[i] = rhs.elements[i];
    }
    else
    {
        this->elements = rhs.elements;
        rhs.elements = rhs.stackElements.data();
    }
    rhs.count = 0;
    rhs.capacity = SMALL_VECTOR_SIZE;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Copy(const Array<TYPE, SMALL_VECTOR_SIZE>& src)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->stackElements.data() == this->elements);
    #endif
    this->GrowTo(src.capacity);
    this->grow = src.grow;
    this->count = src.count;
    IndexT i;
    for (i = 0; i < this->count; i++)
    {
        this->elements[i] = src.elements[i];
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Delete()
{
    this->grow = 16;
    if (this->capacity > 0)
    {
        if (this->elements != this->stackElements.data())
        {
            ArrayFree(this->capacity, this->elements);
        }
        else
        {
            this->DestroyRange(0, this->count);
        }
    }
    this->elements = this->stackElements.data();
    this->count = 0;
    this->capacity = SMALL_VECTOR_SIZE;
}
template<class TYPE, int SMALL_VECTOR_SIZE> void
Array<TYPE, SMALL_VECTOR_SIZE>::Destroy(TYPE* elm)
{
    elm->~TYPE();
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::~Array()
{
    this->Delete();
}
template<class TYPE, int SMALL_VECTOR_SIZE> void
Array<TYPE, SMALL_VECTOR_SIZE>::Realloc(SizeT _capacity, SizeT _grow)
{
    this->Delete();
    this->grow = _grow;
    this->capacity = _capacity;
    this->count = _capacity;
    if (this->capacity > 0)
    {
        this->GrowTo(this->capacity);
    }
    else
    {
        this->elements = 0;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE> void
Array<TYPE, SMALL_VECTOR_SIZE>::operator=(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs)
{
    if (this != &rhs)
    {
        if ((this->capacity > 0) && (rhs.count <= this->capacity))
        {
            n_assert(0 != this->elements);
            this->CopyRange(this->elements, rhs.elements, rhs.count);
            if (rhs.count < this->count)
            {
                this->DestroyRange(rhs.count, this->count);
            }
            this->grow = rhs.grow;
            this->count = rhs.count;
        }
        else
        {
            this->Delete();
            this->Copy(rhs);
        }
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE> void
Array<TYPE, SMALL_VECTOR_SIZE>::operator=(Array<TYPE, SMALL_VECTOR_SIZE>&& rhs) noexcept
{
    if (this != &rhs)
    {
        this->Delete();
        if (rhs.elements != rhs.stackElements.data())
        {
            this->elements = rhs.elements;
            rhs.elements = nullptr;
        }
        else
        {
            this->MoveRange(this->elements, rhs.elements, rhs.count);
        }
        this->grow = rhs.grow;
        this->count = rhs.count;
        this->capacity = rhs.capacity;
        rhs.count = 0;
        rhs.capacity = 0;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE> void
Array<TYPE, SMALL_VECTOR_SIZE>::GrowTo(SizeT newCapacity)
{
    if (newCapacity > SMALL_VECTOR_SIZE)
    {
        TYPE* newArray = ArrayAlloc<TYPE>(newCapacity);
        if (this->elements)
        {
            this->MoveRange(newArray, this->elements, this->count);
            if (this->elements != this->stackElements.data())
                ArrayFree(this->capacity, this->elements);
        }
        this->elements = newArray;
        this->capacity = newCapacity;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Grow()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->grow > 0);
    #endif
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
    this->GrowTo(growToSize);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Move(IndexT fromIndex, IndexT toIndex)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements);
    n_assert(fromIndex < this->count);
    #endif
    if (fromIndex == toIndex)
    {
        return;
    }
    SizeT num = this->count - fromIndex;
    SizeT neededSize = toIndex + num;
    while (neededSize > this->capacity)
    {
        this->Grow();
    }
    if (fromIndex > toIndex)
    {
        this->MoveRange(&this->elements[toIndex], &this->elements[fromIndex], num);
        this->DestroyRange(fromIndex + num - 1, this->count);
    }
    else
    {
        int i;
        for (i = num - 1; i >= 0; --i)
        {
            this->elements[toIndex + i] = this->elements[fromIndex + i];
        }
        this->DestroyRange(fromIndex, toIndex);
    }
    this->count = toIndex + num;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline void
Array<TYPE, SMALL_VECTOR_SIZE>::DestroyRange(IndexT fromIndex, IndexT toIndex)
{
    if constexpr (!std::is_trivially_destructible<TYPE>::value)
    {
        for (IndexT i = fromIndex; i < toIndex; i++)
        {
            this->Destroy(&(this->elements[i]));
        }
    }
    else
    {
        Memory::Clear((void*)&this->elements[fromIndex], sizeof(TYPE) * (toIndex - fromIndex));
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline void
Array<TYPE, SMALL_VECTOR_SIZE>::CopyRange(TYPE* to, TYPE* from, SizeT num)
{
    if constexpr (!std::is_trivially_copyable<TYPE>::value)
    {
        IndexT i;
        for (i = 0; i < num; i++)
        {
            to[i] = from[i];
        }
    }
    else
    {
        Memory::CopyElements(from, to, num);
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline void
Array<TYPE, SMALL_VECTOR_SIZE>::MoveRange(TYPE* to, TYPE* from, SizeT num)
{
    if constexpr (!std::is_trivially_move_assignable<TYPE>::value && std::is_move_assignable<TYPE>::value)
    {
        IndexT i;
        for (i = 0; i < num; i++)
        {
            to[i] = std::move(from[i]);
        }
    }
    else
    {
        Memory::MoveElements(from, to, num);
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline TYPE&
Array<TYPE, SMALL_VECTOR_SIZE>::Get(IndexT index) const
{
#if NEBULA_BOUNDSCHECKS
    n_assert(this->elements != nullptr);
    n_assert(this->capacity > index);
#endif
    return this->elements[index];
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Append(const TYPE& elm)
{
    if (this->count == this->capacity)
    {
        this->Grow();
    }
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements);
    #endif
    this->elements[this->count++] = elm;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Append(TYPE&& elm)
{
    if (this->count == this->capacity)
    {
        this->Grow();
    }
#if NEBULA_BOUNDSCHECKS
    n_assert(this->elements);
#endif
    this->elements[this->count++] = std::move(elm);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::AppendArray(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs)
{
    SizeT neededCapacity = this->count + rhs.count;
    if (neededCapacity > this->capacity)
    {
        this->GrowTo(neededCapacity);
    }
    IndexT i;
    for (i = 0; i < rhs.count; i++)
    {
        this->elements[this->count + i] = rhs.elements[i];
    }
    this->count += rhs.count;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::AppendArray(const TYPE* arr, const SizeT count)
{
    SizeT neededCapacity = this->count + count;
    if (neededCapacity > this->capacity)
    {
        this->GrowTo(neededCapacity);
    }
    IndexT i;
    for (i = 0; i < count; i++)
    {
        this->elements[this->count + i] = arr[i];
    }
    this->count += count;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
TYPE&
Array<TYPE, SMALL_VECTOR_SIZE>::Emplace()
{
    if (this->count == this->capacity)
    {
        this->Grow();
    }
#if NEBULA_BOUNDSCHECKS
    n_assert(this->elements);
#endif
    this->elements[this->count] = TYPE();
    return this->elements[this->count++];
}
template<class TYPE, int SMALL_VECTOR_SIZE>
TYPE*
Array<TYPE, SMALL_VECTOR_SIZE>::EmplaceArray(const SizeT count)
{
    SizeT neededCapacity = this->count + count;
    if (neededCapacity > this->capacity)
    {
        this->GrowTo(neededCapacity);
    }
    IndexT i;
    for (i = 0; i < count; i++)
    {
        this->elements[this->count + i] = TYPE();
    }
    TYPE* first = &this->elements[this->count];
    this->count += count;
    return first;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Reserve(SizeT num)
{
#if NEBULA_BOUNDSCHECKS
    n_assert(num >= 0);
#endif
    SizeT neededCapacity = this->count + num;
    if (neededCapacity > this->capacity)
    {
        this->GrowTo(neededCapacity);
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
const SizeT
Array<TYPE, SMALL_VECTOR_SIZE>::Size() const
{
    return this->count;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
const SizeT
Array<TYPE, SMALL_VECTOR_SIZE>::ByteSize() const
{
    return this->count * sizeof(TYPE);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
const SizeT
Array<TYPE, SMALL_VECTOR_SIZE>::Capacity() const
{
    return this->capacity;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
TYPE&
Array<TYPE, SMALL_VECTOR_SIZE>::operator[](IndexT index) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (index < this->count) && (index >= 0));
    #endif
    return this->elements[index];
}
template<class TYPE, int SMALL_VECTOR_SIZE>
TYPE&
Array<TYPE, SMALL_VECTOR_SIZE>::operator[](IndexT index)
{
#if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (index < this->count) && (index >= 0));
#endif
    return this->elements[index];
}
template<class TYPE, int SMALL_VECTOR_SIZE>
bool
Array<TYPE, SMALL_VECTOR_SIZE>::operator==(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs) const
{
    if (rhs.Size() == this->Size())
    {
        IndexT i;
        SizeT num = this->Size();
        for (i = 0; i < num; i++)
        {
            if (!(this->elements[i] == rhs.elements[i]))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
bool
Array<TYPE, SMALL_VECTOR_SIZE>::operator!=(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs) const
{
    return !(*this == rhs);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
TYPE&
Array<TYPE, SMALL_VECTOR_SIZE>::Front() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (this->count > 0));
    #endif
    return this->elements[0];
}
template<class TYPE, int SMALL_VECTOR_SIZE>
TYPE&
Array<TYPE, SMALL_VECTOR_SIZE>::Back() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (this->count > 0));
    #endif
    return this->elements[this->count - 1];
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::push_back(const TYPE& item)
{
    this->Append(item);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
bool
Array<TYPE, SMALL_VECTOR_SIZE>::IsEmpty() const
{
    return (this->count == 0);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
bool
Array<TYPE, SMALL_VECTOR_SIZE>::IsValidIndex(IndexT index) const
{
    return this->elements && (index < this->count) && (index >= 0);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::EraseIndex(IndexT index)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (index < this->count) && (index >= 0));
    #endif
    if (index == (this->count - 1))
    {
        this->EraseBack();
    }
    else
    {
        this->Move(index + 1, index);
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::EraseIndexSwap(IndexT index)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (index < this->count) && (index >= 0));
    #endif
    IndexT lastElementIndex = this->count - 1;
    if (index < lastElementIndex)
    {
        if constexpr (!std::is_trivially_move_assignable<TYPE>::value)
            this->elements[index] = std::move(this->elements[lastElementIndex]);
        else
            this->elements[index] = this->elements[lastElementIndex];
    }
    this->count--;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator
Array<TYPE, SMALL_VECTOR_SIZE>::Erase(typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator iter)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (iter >= this->elements) && (iter < (this->elements + this->count)));
    #endif
    this->EraseIndex(IndexT(iter - this->elements));
    return iter;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator
Array<TYPE, SMALL_VECTOR_SIZE>::EraseSwap(typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator iter)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->elements && (iter >= this->elements) && (iter < (this->elements + this->count)));
    #endif
    this->EraseIndexSwap(IndexT(iter - this->elements));
    return iter;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::EraseRange(IndexT start, IndexT end)
{
    n_assert(end >= start);
    n_assert(end <= this->count);
    if (start == end)
        this->EraseIndex(start);
    else
    {
        this->DestroyRange(start, end);
        SizeT numMove = this->count - end;
        this->MoveRange(&this->elements[start], &this->elements[end], numMove);
        this->count -= end - start;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::EraseBack()
{
    n_assert(this->count > 0);
    if constexpr (!std::is_trivially_destructible<TYPE>::value)
        this->Destroy(&(this->elements[this->count - 1]));
    this->count--;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::EraseFront()
{
    this->EraseIndex(0);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline TYPE
Array<TYPE, SMALL_VECTOR_SIZE>::PopFront()
{
#if NEBULA_BOUNDSCHECKS
    n_assert(this->count > 0);
#endif
    TYPE ret = std::move(this->elements[0]);
    this->EraseIndex(0);
    return ret;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline TYPE
Array<TYPE, SMALL_VECTOR_SIZE>::PopBack()
{
#if NEBULA_BOUNDSCHECKS
    n_assert(this->count > 0);
#endif
    this->count--;
    return std::move(this->elements[this->count]);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Insert(IndexT index, const TYPE& elm)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(index <= this->count && (index >= 0));
    #endif
    if (index == this->count)
    {
        this->Append(elm);
    }
    else
    {
        this->Move(index, index + 1);
        this->elements[index] = elm;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Clear()
{
    if (this->count > 0)
    {
        this->DestroyRange(0, this->count);
        this->count = 0;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Reset()
{
    this->count = 0;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Free()
{
    this->Delete();
    this->grow = 16;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator
Array<TYPE, SMALL_VECTOR_SIZE>::Begin() const
{
    return this->elements;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::ConstIterator
Array<TYPE, SMALL_VECTOR_SIZE>::ConstBegin() const
{
    return static_cast<Array<TYPE, SMALL_VECTOR_SIZE>::ConstIterator>(this->elements);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator
Array<TYPE, SMALL_VECTOR_SIZE>::End() const
{
    return this->elements + this->count;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::ConstIterator
Array<TYPE, SMALL_VECTOR_SIZE>::ConstEnd() const
{
    return static_cast<Array<TYPE, SMALL_VECTOR_SIZE>::ConstIterator>(this->elements + this->count);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator
Array<TYPE, SMALL_VECTOR_SIZE>::Find(const TYPE& elm, const IndexT start) const
{
    n_assert(start <= this->count);
    IndexT index;
    for (index = start; index < this->count; index++)
    {
        if (this->elements[index] == elm)
        {
            return &(this->elements[index]);
        }
    }
    return 0;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
IndexT
Array<TYPE, SMALL_VECTOR_SIZE>::FindIndex(const TYPE& elm, const IndexT start) const
{
    n_assert(start <= this->count);
    IndexT index;
    for (index = start; index < this->count; index++)
    {
        if (this->elements[index] == elm)
        {
            return index;
        }
    }
    return InvalidIndex;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
template<typename ...ELEM_TYPE>
inline void
Array<TYPE, SMALL_VECTOR_SIZE>::Append(const TYPE& first, const ELEM_TYPE&... elements)
{
    const int size = sizeof...(elements) + 1;
    this->Reserve(size);
    TYPE res[size] = { first, elements... };
    for (IndexT i = 0; i < size; i++)
    {
        this->elements[this->count++] = res[i];
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
template<typename KEYTYPE>
inline IndexT
Array<TYPE, SMALL_VECTOR_SIZE>::FindIndex(typename std::enable_if<true, const KEYTYPE&>::type elm, const IndexT start) const
{
    n_assert(start <= this->count);
    IndexT index;
    for (index = start; index < this->count; index++)
    {
        if (this->elements[index] == elm)
        {
            return index;
        }
    }
    return InvalidIndex;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Fill(IndexT first, SizeT num, const TYPE& elm)
{
    if ((first + num) > this->count)
    {
        this->GrowTo(first + num);
        this->count = first + num;
    }
    IndexT i;
    for (i = first; i < (first + num); i++)
    {
        this->elements[i] = elm;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>
Array<TYPE, SMALL_VECTOR_SIZE>::Difference(const Array<TYPE, SMALL_VECTOR_SIZE>& rhs)
{
    Array<TYPE, SMALL_VECTOR_SIZE> diff;
    IndexT i;
    SizeT num = rhs.Size();
    for (i = 0; i < num; i++)
    {
        if (0 == this->Find(rhs[i]))
        {
            diff.Append(rhs[i]);
        }
    }
    return diff;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Sort()
{
    std::sort(this->Begin(), this->End());
}
template <class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::QuickSort()
{
    std::qsort(
        this->Begin(),
        this->Size(),
        sizeof(TYPE),
        [](const void* a, const void* b)
        {
            TYPE arg1 = *static_cast<const TYPE*>(a);
            TYPE arg2 = *static_cast<const TYPE*>(b);
            return (arg1 > arg2) - (arg1 < arg2);
        }
    );
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Util::Array<TYPE, SMALL_VECTOR_SIZE>::SortWithFunc(bool (*func)(const TYPE& lhs, const TYPE& rhs))
{
    std::sort(this->Begin(), this->End(), func);
}
template <class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::QuickSortWithFunc(int (*func)(const void* lhs, const void* rhs))
{
    std::qsort(
        this->Begin(),
        this->Size(),
        sizeof(TYPE),
        func
    );
}
template<class TYPE, int SMALL_VECTOR_SIZE>
IndexT
Array<TYPE, SMALL_VECTOR_SIZE>::BinarySearchIndex(const TYPE& elm) const
{
    SizeT num = this->Size();
    if (num > 0)
    {
        IndexT half;
        IndexT lo = 0;
        IndexT hi = num - 1;
        IndexT mid;
        while (lo <= hi)
        {
            if (0 != (half = num/2))
            {
                mid = lo + ((num & 1) ? half : (half - 1));
                if (elm < this->elements[mid])
                {
                    hi = mid - 1;
                    num = num & 1 ? half : half - 1;
                }
                else if (elm > this->elements[mid])
                {
                    lo = mid + 1;
                    num = half;
                }
                else
                {
                    return mid;
                }
            }
            else if (0 != num)
            {
                if (elm != this->elements[lo])
                {
                    return InvalidIndex;
                }
                else
                {
                    return lo;
                }
            }
            else
            {
                break;
            }
        }
    }
    return InvalidIndex;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
template<typename KEYTYPE> inline IndexT
Array<TYPE, SMALL_VECTOR_SIZE>::BinarySearchIndex(typename std::enable_if<true, const KEYTYPE&>::type elm) const
{
    SizeT num = this->Size();
    if (num > 0)
    {
        IndexT half;
        IndexT lo = 0;
        IndexT hi = num - 1;
        IndexT mid;
        while (lo <= hi)
        {
            if (0 != (half = num / 2))
            {
                mid = lo + ((num & 1) ? half : (half - 1));
                if (this->elements[mid] > elm)
                {
                    hi = mid - 1;
                    num = num & 1 ? half : half - 1;
                }
                else if (this->elements[mid] < elm)
                {
                    lo = mid + 1;
                    num = half;
                }
                else
                {
                    return mid;
                }
            }
            else if (0 != num)
            {
                if (this->elements[lo] != elm)
                {
                    return InvalidIndex;
                }
                else
                {
                    return lo;
                }
            }
            else
            {
                break;
            }
        }
    }
    return InvalidIndex;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::Resize(SizeT num)
{
    if (num < this->count)
    {
        this->DestroyRange(num, this->count);
    }
    else if (num > this->capacity)
    {
        this->GrowTo(num);
    }
    this->count = num;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
template<typename ...ARGS>
void Array<TYPE, SMALL_VECTOR_SIZE>::Resize(SizeT num, ARGS... args)
{
    if (num < this->count)
    {
        this->DestroyRange(num, this->count);
    }
    else if (num > this->capacity)
    {
        SizeT oldCapacity = this->capacity;
        this->GrowTo(num);
        for (IndexT i = oldCapacity; i < this->capacity; i++)
            this->elements[i] = TYPE(args...);
    }
    this->count = num;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline void Array<TYPE, SMALL_VECTOR_SIZE>::Extend(SizeT num)
{
    if (num > this->capacity)
    {
        this->GrowTo(num);
        this->count = num;
    }
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::clear() noexcept
{
    this->Clear();
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline void
Array<TYPE, SMALL_VECTOR_SIZE>::Fit()
{
    TYPE* newArray = ArrayAlloc<TYPE>(this->count);
    if (this->elements)
    {
        this->MoveRange(newArray, this->elements, this->count);
        if (this->elements != this->stackElements.data())
            ArrayFree(this->capacity, this->elements);
    }
    this->elements = newArray;
    this->capacity = this->count;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
inline constexpr SizeT
Array<TYPE, SMALL_VECTOR_SIZE>::TypeSize() const
{
    return sizeof(TYPE);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
size_t
Array<TYPE, SMALL_VECTOR_SIZE>::size() const
{
    return this->count;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator
Array<TYPE, SMALL_VECTOR_SIZE>::begin() const
{
    return this->elements;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
typename Array<TYPE, SMALL_VECTOR_SIZE>::Iterator
Array<TYPE, SMALL_VECTOR_SIZE>::end() const
{
    return this->elements + this->count;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
void
Array<TYPE, SMALL_VECTOR_SIZE>::resize(size_t s)
{
    if (static_cast<SizeT>(s) > this->capacity)
    {
        this->GrowTo(static_cast<SizeT>(s));
    }
    this->count = static_cast<SizeT>(s);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
bool
Array<TYPE, SMALL_VECTOR_SIZE>::IsSorted() const
{
    if (this->count > 1)
    {
        IndexT i;
        for (i = 0; i < this->count - 1; i++)
        {
            if (this->elements[i] > this->elements[i + 1])
            {
                return false;
            }
        }
    }
    return true;
}
template<class TYPE, int SMALL_VECTOR_SIZE>
IndexT
Array<TYPE, SMALL_VECTOR_SIZE>::InsertAtEndOfIdenticalRange(IndexT startIndex, const TYPE& elm)
{
    IndexT i = startIndex + 1;
    for (; i < this->count; i++)
    {
        if (this->elements[i] != elm)
        {
            this->Insert(i, elm);
            return i;
        }
    }
    this->Append(elm);
    return (this->Size() - 1);
}
template<class TYPE, int SMALL_VECTOR_SIZE>
IndexT
Array<TYPE, SMALL_VECTOR_SIZE>::InsertSorted(const TYPE& elm)
{
    SizeT num = this->Size();
    if (num == 0)
    {
        this->Append(elm);
        return this->Size() - 1;
    }
    else
    {
        IndexT half;
        IndexT lo = 0;
        IndexT hi = num - 1;
        IndexT mid;
        while (lo <= hi)
        {
            if (0 != (half = num/2))
            {
                mid = lo + ((num & 1) ? half : (half - 1));
                if (elm < this->elements[mid])
                {
                    hi = mid - 1;
                    num = num & 1 ? half : half - 1;
                }
                else if (elm > this->elements[mid])
                {
                    lo = mid + 1;
                    num = half;
                }
                else
                {
                    return this->InsertAtEndOfIdenticalRange(mid, elm);
                }
            }
            else if (0 != num)
            {
                if (elm < this->elements[lo])
                {
                    this->Insert(lo, elm);
                    return lo;
                }
                else if (elm > this->elements[lo])
                {
                    this->Insert(lo + 1, elm);
                    return lo + 1;
                }
                else
                {
                    return this->InsertAtEndOfIdenticalRange(lo, elm);
                }
            }
            else
            {
                #if NEBULA_BOUNDSCHECKS
                n_assert(0 == lo);
                #endif
                this->Insert(lo, elm);
                return lo;
            }
        }
        if (elm < this->elements[lo])
        {
            this->Insert(lo, elm);
            return lo;
        }
        else if (elm > this->elements[lo])
        {
            this->Insert(lo + 1, elm);
            return lo + 1;
        }
        else
        {
        }
    }
    n_error("Array::InsertSorted: Не может быть!");
    return InvalidIndex;
}
template<class TYPE, int STACK_SIZE>
using StackArray = Array<TYPE, STACK_SIZE>;
}