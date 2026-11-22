#pragma once
#include "core/types.h"
#include "util/array.h"
#include <tuple>
#include "tupleutility.h"
namespace Util
{
template <class ... TYPES>
class ArrayAllocator
{
public:
    ArrayAllocator();
    ArrayAllocator(ArrayAllocator<TYPES...>&& rhs);
    ArrayAllocator(const ArrayAllocator<TYPES...>& rhs);
    ~ArrayAllocator();
    void operator=(const ArrayAllocator<TYPES...>& rhs);
    void operator=(ArrayAllocator<TYPES...>&& rhs);
    uint32_t Alloc();
    void EraseIndex(const uint32_t id);
    void EraseIndexSwap(const uint32_t id);
    void EraseRange(const uint32_t start, const uint32_t end);
    template <int MEMBER>
    tuple_array_t<MEMBER, TYPES...>& Get(const uint32_t index);
    template <int MEMBER>
    const tuple_array_t<MEMBER, TYPES...>& ConstGet(const uint32_t index) const;
    template <int MEMBER>
    void Set(const uint32_t index, const tuple_array_t<MEMBER, TYPES...>& type);
    template <int MEMBER>
    const Util::Array<tuple_array_t<MEMBER, TYPES...>>& GetArray() const;
    template <int MEMBER>
    Util::Array<tuple_array_t<MEMBER, TYPES...>>& GetArray();
    void Set(const uint32_t index, TYPES...);
    const uint32_t Size() const;
    void Reserve(uint32_t size);
    void SetSize(uint32_t size);
    void Clear();
    void UpdateSize();
protected:
    uint32_t size;
    std::tuple<Util::Array<TYPES>...> objects;
};
template<class ... TYPES>
inline ArrayAllocator<TYPES...>::ArrayAllocator() :
    size(0)
{
}
template<class ...TYPES>
inline ArrayAllocator<TYPES...>::ArrayAllocator(ArrayAllocator<TYPES...>&& rhs)
{
    this->objects = rhs.objects;
    this->size = rhs.size;
    rhs.Clear();
}
template<class ...TYPES>
inline ArrayAllocator<TYPES...>::ArrayAllocator(const ArrayAllocator<TYPES...>& rhs)
{
    this->objects = rhs.objects;
    this->size = rhs.size;
}
template<class ...TYPES>
inline ArrayAllocator<TYPES...>::~ArrayAllocator()
{
}
template<class ...TYPES>
inline void
ArrayAllocator<TYPES...>::operator=(const ArrayAllocator<TYPES...>& rhs)
{
    this->objects = rhs.objects;
    this->size = rhs.size;
}
template<class ...TYPES>
inline void
ArrayAllocator<TYPES...>::operator=(ArrayAllocator<TYPES...>&& rhs)
{
    this->objects = rhs.objects;
    this->size = rhs.size;
    rhs.Clear();
}
template<class ...TYPES>
inline uint32_t ArrayAllocator<TYPES...>::Alloc()
{
    alloc_for_each_in_tuple(this->objects);
    return this->size++;
}
template<class ...TYPES> inline void
ArrayAllocator<TYPES...>::EraseIndex(const uint32_t id)
{
    erase_index_for_each_in_tuple(this->objects, id);
    this->size--;
}
template<class ...TYPES> inline void
ArrayAllocator<TYPES...>::EraseIndexSwap(const uint32_t id)
{
    erase_index_swap_for_each_in_tuple(this->objects, id);
    this->size--;
}
template<class ...TYPES> inline void
ArrayAllocator<TYPES...>::EraseRange(const uint32_t start, const uint32_t end)
{
    erase_range_for_each_in_tuple(this->objects, start, end);
    this->size -= (end + 1) - start;
}
template<class ...TYPES>
inline const uint32_t
ArrayAllocator<TYPES...>::Size() const
{
    return this->size;
}
template<class ...TYPES>
inline void
ArrayAllocator<TYPES...>::Reserve(uint32_t num)
{
    reserve_for_each_in_tuple(this->objects, num);
}
template<class ...TYPES>
inline void
ArrayAllocator<TYPES...>::SetSize(uint32_t size)
{
    set_size_for_each_in_tuple(this->objects, size);
    this->size = size;
}
template<class ...TYPES>
inline void
ArrayAllocator<TYPES...>::Clear()
{
    clear_for_each_in_tuple(this->objects);
    this->size = 0;
}
template<class ...TYPES>
template<int MEMBER>
inline tuple_array_t<MEMBER, TYPES...>&
ArrayAllocator<TYPES...>::Get(const uint32_t index)
{
    return std::get<MEMBER>(this->objects)[index];
}
template<class ...TYPES>
template<int MEMBER>
inline const tuple_array_t<MEMBER, TYPES...>&
ArrayAllocator<TYPES...>::ConstGet(const uint32_t index) const
{
    return std::get<MEMBER>(this->objects)[index];
}
template<class ...TYPES>
template<int MEMBER>
inline void
ArrayAllocator<TYPES...>::Set(const uint32_t index, const tuple_array_t<MEMBER, TYPES...>& type)
{
    std::get<MEMBER>(this->objects)[index] = type;
}
template<class ...TYPES>
template<int MEMBER>
inline const Util::Array<tuple_array_t<MEMBER, TYPES...>>&
ArrayAllocator<TYPES...>::GetArray() const
{
    return std::get<MEMBER>(this->objects);
}
template<class ...TYPES>
template<int MEMBER>
inline Util::Array<tuple_array_t<MEMBER, TYPES...>>&
ArrayAllocator<TYPES...>::GetArray()
{
    return std::get<MEMBER>(this->objects);
}
template<class ...TYPES> void
ArrayAllocator<TYPES...>::Set(const uint32_t index, TYPES... values)
{
    set_for_each_in_tuple(this->objects, index, values...);
}
template<class ...TYPES> void
ArrayAllocator<TYPES...>::UpdateSize()
{
    this->size = this->GetArray<0>().Size();
}
}