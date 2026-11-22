#pragma once
#include "id.h"
#include "idpool.h"
#include "util/array.h"
#include "threading/criticalsection.h"
#include <tuple>
#include <utility>
#include "util/tupleutility.h"
#include "util/arrayallocator.h"
#include "util/arrayallocatorsafe.h"
namespace Ids
{
template<class ... TYPES>
class IdAllocator : public Util::ArrayAllocator<TYPES...>
{
public:
    IdAllocator(Ids::Id32 maxid = 0xFFFFFFFF) : maxId(maxid) {};
    Ids::Id32 Alloc()
    {
        Ids::Id32 index;
        if (this->freeIds.Size() > 0)
        {
            index = this->freeIds.Back();
            this->freeIds.EraseBack();
        }
        else
        {
            index = Util::ArrayAllocator<TYPES...>::Alloc();
            n_assert2(this->maxId > index, "Превышено максимальное количество выделений!\n");
        }
        return index;
    }
    void Dealloc(Ids::Id32 index)
    {
        this->freeIds.Append(index);
    }
    Util::Array<Ids::Id32>& FreeIds()
    {
        return this->freeIds;
    }
    const Ids::Id32 Size() const
    {
        return this->size;
    }
private:
    Ids::Id32 maxId = 0xFFFFFFFF;
    Util::Array<Ids::Id32> freeIds;
};
#define _DECL_ACQUIRE_RELEASE(ty) \
    bool ty##Acquire(const ty id); \
    void ty##Release(const ty id); \
    struct ty##Lock \
    { \
        ty##Lock(const ty element) : element(element) { this->didAcquire = ty##Acquire(this->element); } \
        ~ty##Lock() { if (this->didAcquire) ty##Release(this->element); } \
    private: \
        bool didAcquire; \
        ty element; \
    };
#define _IMPL_ACQUIRE_RELEASE(ty, allocator) \
    bool ty##Acquire(const ty id) { return allocator.Acquire(id.id); } \
    void ty##Release(const ty id) { allocator.Release(id.id); }
template<int MAX_ALLOCS, class... TYPES>
class IdAllocatorSafe : public Util::ArrayAllocatorSafe<MAX_ALLOCS, TYPES...>
{
public:
    IdAllocatorSafe()
    {
    };
    Ids::Id32 Alloc()
    {
        this->allocationLock.Lock();
        Ids::Id32 index;
        if (this->freeIds.Size() > 0)
        {
            index = this->freeIds.Back();
            this->owners[index] = Threading::Thread::GetMyThreadId();
            this->freeIds.EraseBack();
        }
        else
        {
            alloc_for_each_in_tuple(this->objects);
            index = this->size++;
            this->owners.Append(Threading::Thread::GetMyThreadId());
            this->generations.Append(0);
            n_assert2(MAX_ALLOCS > index, "Превышено максимальное количество выделений!\n");
        }
        this->allocationLock.Unlock();
        Ids::Id8 generation = this->generations[index];
        return index;
    }
    void Dealloc(Ids::Id32 index)
    {
        this->allocationLock.Lock();
        this->freeIds.Append(index);
        this->generations[index]++;
        this->allocationLock.Unlock();
    }
private:
    Util::Array<Ids::Id32> freeIds;
    Util::Array<Ids::Id8> generations;
};
}