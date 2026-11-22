#pragma once
#include "types.h"
#include "util/pinnedarray.h"
#include "threading/readwritelock.h"
#include "threading/spinlock.h"
#include <tuple>
#include "tupleutility.h"
#include "ids/id.h"
#define __Lock(name, element) auto __allocator_lock_##name##__ = Util::AllocatorLock(&name, element);
#define __LockName(allocator, name, element) auto __allocator_lock_##name##__ = Util::AllocatorLock(allocator, element);
namespace Util
{
template<class T>
struct AllocatorLock
{
    AllocatorLock(T* allocator, uint32_t element)
        : element(element)
        , allocator(allocator)
    {
        this->didAcquire = this->allocator->Acquire(this->element);
    };
    ~AllocatorLock()
    {
        if (this->didAcquire)
            this->allocator->Release(this->element);
    }
    bool didAcquire;
    uint32_t element;
    T* allocator;
};
template <uint MAX_ALLOCS = 0xFFFF, class ... TYPES>
class ArrayAllocatorSafe
{
public:
    ArrayAllocatorSafe();
    ArrayAllocatorSafe(ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>&& rhs);
    ArrayAllocatorSafe(const ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>& rhs);
    ~ArrayAllocatorSafe();
    void operator=(const ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>& rhs);
    void operator=(ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>&& rhs);
    uint32_t Alloc();
    void EraseIndex(const uint32_t id);
    void EraseIndexSwap(const uint32_t id);
    template <int MEMBER>
    tuple_array_t<MEMBER, TYPES...>& Get(const uint32_t index);
    template <int MEMBER>
    const tuple_array_t<MEMBER, TYPES...>& ConstGet(const uint32_t index) const;
    template <int MEMBER>
    const tuple_array_t<MEMBER, TYPES...>& Get(const uint32_t index) const;
    template <int MEMBER>
    void Set(const uint32_t index, const tuple_array_t<MEMBER, TYPES...>& type);
    template <int MEMBER>
    const Util::Array<tuple_array_t<MEMBER, TYPES...>>& GetArray() const;
    template <int MEMBER>
    Util::Array<tuple_array_t<MEMBER, TYPES...>>& GetArray();
    void Set(const uint32_t index, TYPES...);
    const uint32_t Size() const;
    void Reserve(uint32_t size);
    void Clear();
    void UpdateSize();
    void TryAcquire(const uint32_t index);
    bool Acquire(const uint32_t index);
    void Release(const uint32_t index);
protected:
    uint32_t size;
    std::tuple<Util::PinnedArray<MAX_ALLOCS, TYPES>...> objects;
    Util::PinnedArray<MAX_ALLOCS, Threading::ThreadId> owners;
    Threading::Spinlock allocationLock;
};
template<uint MAX_ALLOCS, class ...TYPES>
inline
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::ArrayAllocatorSafe() :
    size(0)
{
}
template<uint MAX_ALLOCS, class ...TYPES>
inline
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::ArrayAllocatorSafe(ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>&& rhs)
{
    this->allocationLock.Lock();
    rhs.allocationLock.Lock();
    this->objects = rhs.objects;
    this->size = rhs.size;
    rhs.Clear();
    this->allocationLock.Unlock();
    rhs.allocationLock.Unlock();
}
template<uint MAX_ALLOCS, class ...TYPES>
inline
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::ArrayAllocatorSafe(const ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>& rhs)
{
    this->allocationLock.Lock();
    this->objects = rhs.objects;
    this->size = rhs.size;
    this->allocationLock.Unlock();
}
template<uint MAX_ALLOCS, class ...TYPES>
inline
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::~ArrayAllocatorSafe()
{
}
template<uint MAX_ALLOCS, class ...TYPES>
inline void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::operator=(const ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>& rhs)
{
    this->allocationLock.Lock();
    this->objects = rhs.objects;
    this->size = rhs.size;
    this->allocationLock.Unlock();
}
template<uint MAX_ALLOCS, class ...TYPES>
inline void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::operator=(ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>&& rhs)
{
    this->allocationLock.Lock();
    rhs.allocationLock.Lock();
    this->objects = rhs.objects;
    this->size = rhs.size;
    rhs.Clear();
    this->allocationLock.Unlock();
    rhs.allocationLock.Unlock();
}
template<uint MAX_ALLOCS, class ...TYPES>
inline uint32_t
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Alloc()
{
    this->allocationLock.Lock();
    alloc_for_each_in_tuple(this->objects);
    auto i = this->size++;
    this->owners.Append(Threading::Thread::GetMyThreadId());
    this->allocationLock.Unlock();
    return i;
}
template<uint MAX_ALLOCS, class ...TYPES>
inline void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::EraseIndex(const uint32_t id)
{
    n_assert(this->owners[id] == Threading::Thread::GetMyThreadId());
    this->allocationLock.Lock();
    erase_index_for_each_in_tuple(this->objects, id);
    this->allocationLock.Unlock();
    this->size--;
}
template<uint MAX_ALLOCS, class ...TYPES>
inline void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::EraseIndexSwap(const uint32_t id)
{
    n_assert(this->owners[id] == Threading::Thread::GetMyThreadId());
    this->allocationLock.Lock();
    erase_index_swap_for_each_in_tuple(this->objects, id);
    this->allocationLock.Unlock();
    this->size--;
}
template<uint MAX_ALLOCS, class ...TYPES>
inline const uint32_t
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Size() const
{
    n_assert2(this->numReaders > 0, "Размер требует блокировки чтения");
    return this->size;
}
template<uint MAX_ALLOCS, class ...TYPES>
inline void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Reserve(uint32_t num)
{
    this->allocationLock.Lock();
    reserve_for_each_in_tuple(this->objects, num);
    this->allocationLock.Unlock();
}
template<uint MAX_ALLOCS, class ...TYPES>
inline void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Clear()
{
    this->allocationLock.Enter();
    clear_for_each_in_tuple(this->objects);
    this->size = 0;
    this->allocationLock.Leave();
}
template<uint MAX_ALLOCS, class ...TYPES>
template<int MEMBER>
inline tuple_array_t<MEMBER, TYPES...>&
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Get(const uint32_t index)
{
    n_assert(this->owners[index] == Threading::Thread::GetMyThreadId());
    return std::get<MEMBER>(this->objects)[index];
}
template<uint MAX_ALLOCS, class ...TYPES>
template<int MEMBER>
inline const tuple_array_t<MEMBER, TYPES...>&
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::ConstGet(const uint32_t index) const
{
    n_assert(this->owners[index] == Threading::Thread::GetMyThreadId() || this->owners[index] == Threading::InvalidThreadId);
    return std::get<MEMBER>(this->objects)[index];
}
template<uint MAX_ALLOCS, class ...TYPES>
template<int MEMBER>
inline const tuple_array_t<MEMBER, TYPES...>&
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Get(const uint32_t index) const
{
    n_assert(this->owners[index] == Threading::Thread::GetMyThreadId() || this->owners[index] == Threading::InvalidThreadId);
    return std::get<MEMBER>(this->objects)[index];
}
template<uint MAX_ALLOCS, class ...TYPES>
template<int MEMBER>
inline void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Set(const uint32_t index, const tuple_array_t<MEMBER, TYPES...>& type)
{
    n_assert(this->owners[index] == Threading::Thread::GetMyThreadId());
    std::get<MEMBER>(this->objects)[index] = type;
}
template<uint MAX_ALLOCS, class ...TYPES>
template<int MEMBER>
inline const Util::Array<tuple_array_t<MEMBER, TYPES...>>&
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::GetArray() const
{
    return std::get<MEMBER>(this->objects);
}
template<uint MAX_ALLOCS, class ...TYPES>
template<int MEMBER>
inline Util::Array<tuple_array_t<MEMBER, TYPES...>>&
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::GetArray()
{
    return std::get<MEMBER>(this->objects);
}
template<uint MAX_ALLOCS, class ...TYPES>
void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Set(const uint32_t index, TYPES... values)
{
    set_for_each_in_tuple(this->objects, index, values...);
}
template<uint MAX_ALLOCS, class ...TYPES>
void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::UpdateSize()
{
    this->size = this->GetArray<0>().Size();
}
template<uint MAX_ALLOCS, class ...TYPES>
void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::TryAcquire(const uint32_t index)
{
    Threading::ThreadId myThread = Threading::Thread::GetMyThreadId();
    Threading::ThreadId currentThread = Threading::Interlocked::CompareExchange((volatile Threading::ThreadIdStorage*)&this->owners[index], myThread, Threading::InvalidThreadId);
    n_assert(currentThread == Threading::InvalidThreadId);
}
template<uint MAX_ALLOCS, class ...TYPES>
bool
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Acquire(const uint32_t index)
{
    Threading::ThreadId myThread = Threading::Thread::GetMyThreadId();
    if (this->owners[index] == myThread)
        return false;
    while (Threading::Interlocked::CompareExchange((volatile Threading::ThreadIdStorage*)&this->owners[index], myThread, Threading::InvalidThreadId) != Threading::InvalidThreadId)
    {
        Threading::Thread::YieldThread();
    };
    return true;
}
template<uint MAX_ALLOCS, class ...TYPES>
void
ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>::Release(const uint32_t index)
{
    n_assert(this->owners[index] == Threading::Thread::GetMyThreadId());
    Threading::Interlocked::Exchange((volatile Threading::ThreadIdStorage*)&this->owners[index], Threading::InvalidThreadId);
}
}