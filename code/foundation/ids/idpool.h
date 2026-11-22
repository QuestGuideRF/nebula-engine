#pragma once
#include <cstdint>
#include "core/types.h"
#include "util/array.h"
#include "math/scalar.h"
#include <functional>
namespace Ids
{
class IdPool
{
public:
    IdPool();
    IdPool(const uint max, const uint grow = 512);
    ~IdPool();
    uint Alloc();
    void Dealloc(uint id);
    void Reserve(uint numIds);
    uint GetNumUsed() const;
    uint GetNumFree() const;
    void ForEachFree(const std::function<void(uint, uint)> fun, SizeT num);
    void Move(uint lhs, uint rhs);
    const uint GetGrow() const;
private:
    Util::Array<uint> free;
    uint maxId;
    uint grow;
};
inline
IdPool::IdPool() :
    maxId(0xFFFFFFFF),
    grow(512)
{
}
inline
IdPool::IdPool(const uint max, const uint grow) :
    maxId(max),
    grow(grow)
{
}
inline
IdPool::~IdPool()
{
}
inline uint
IdPool::Alloc()
{
    if (this->free.Size() == 0)
    {
        SizeT growTo = Math::min(this->maxId, this->grow);
        SizeT oldCapacity = this->free.Capacity();
        n_assert2((uint)(oldCapacity + growTo) < this->maxId, "Бассейн полон! Будьте осторожны с тем, сколько вы выделяете!\n");
        this->free.Reserve(oldCapacity + growTo);
        IndexT i;
        for (i = this->free.Capacity()-1; i >= oldCapacity; i--)
        {
            this->free.Append(this->maxId - i);
        }
    }
    uint id = this->maxId - this->free.Back();
    this->free.EraseIndex(this->free.Size() - 1);
    return id;
}
inline void
IdPool::Dealloc(uint id)
{
    this->free.Append(this->maxId - id);
}
inline void
IdPool::Reserve(uint numIds)
{
    this->maxId = numIds;
    this->free.Reserve(numIds);
    for (int i = numIds - 1; i >= 0; i--)
    {
        this->free.Append(this->maxId - i);
    }
}
inline uint
IdPool::GetNumUsed() const
{
    return this->free.Capacity() - this->free.Size();
}
inline uint
IdPool::GetNumFree() const
{
    return this->free.Size();
}
inline void
IdPool::ForEachFree(const std::function<void(uint, uint)> fun, SizeT num)
{
    SizeT size = this->free.Size();
    for (IndexT i = size - 1; i >= 0; i--)
    {
        const uint id = this->maxId - this->free[i];
        if (id < (uint)num)
        {
            fun(id, i);
            num--;
        }
    }
}
inline void
IdPool::Move(uint idx, uint id)
{
    this->free.Append(this->maxId - id);
    this->free.EraseIndex(idx);
}
inline const uint
IdPool::GetGrow() const
{
    return this->grow;
}
}